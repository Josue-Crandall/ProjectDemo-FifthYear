/*
	Alice v.08 a chat client and server.
	Copyright (C) 2020 Josue Crandall

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#define POLL_EVENT_QUEUE_SIZE               (128)

static Ret PollerInit(Poller *poller, int timeoutMiliSec) {
	memset(poller, 0, sizeof(*poller));

	CHECK(PipeInit(&poller->pip, NULL));
	CHECK(PollFDsInit(&poller->fds, POLL_EVENT_QUEUE_SIZE));
	CHECK(PollTaskTrackersInit(&poller->trackers, POLL_EVENT_QUEUE_SIZE));

	struct pollfd pipeFD;
	pipeFD.fd = PipeGetReadEnd(&poller->pip);
	pipeFD.events = POLLIN | POLLERR | POLLHUP;
	CHECK(PollFDsPush(&poller->fds, pipeFD));

	poller->timeoutMiliSec = timeoutMiliSec;

	return 0;

FAILED:
	PollerDe(poller);
	memset(poller, 0, sizeof(*poller));
	return -1;	
}
static void PollerDe(Poller *poller) {
	if(PollFDsAlive(&poller->fds)) {
		PollTaskTrackersDe(&poller->trackers);
		PollFDsDe(&poller->fds);
		PipeDe(&poller->pip);
	}
}
static void PollerSetTimeout(Poller *poller, int timeoutMiliSec) {
	poller->timeoutMiliSec = timeoutMiliSec;
}
static Ret PollerPush(Poller *poller, PTask **task) {
	CHECK(PipeWrite(&poller->pip, (void *)&task, sizeof(PTask **)));
	return 0;
FAILED:
	(*task)->de(task);
	return -1;
}
static void PollerStop(Poller *poller) { PipeCloseWriteEnd(&poller->pip); }
static Ret TaskClearingPredicate(void *self, PollTaskTracker *tracker) { void *ign = self; return !tracker->task; }
static Ret FDsClearingPredicate(void *self, struct pollfd *pollfd) { void *ign = self; return !pollfd->events; }
static PollFDsPred POLL_CLEAR_PRED2 = { FDsClearingPredicate };
static PollTaskTrackersPred POLL_CLEAR_PRED = { TaskClearingPredicate };
static void PollerLoop(Poller *poller) {
	while(1) {
		static_assert(sizeof(nfds_t) >= sizeof(usize), "casting len type");
		int numRdy = poll(PollFDsData(&poller->fds), PollFDsSize(&poller->fds), poller->timeoutMiliSec);
		if(numRdy < 0) {
			if(EINTR == errno) { continue; }
			else {
				DEBUG_LOG("poll failed\n");
				goto FAILED;
			}
		}

		#ifdef JC_DEBUG
			fprintf(stderr, "Poller spin with %d ready\n", numRdy);
		#endif

		if(numRdy) {
			struct pollfd *fd = PollFDsData(&poller->fds);
			if(fd->events & fd->revents) {
				while(1) {
					struct PollTaskTracker tracker;
					usize ign;
					Ret res = PipeRead(&poller->pip, (void *)&tracker.task, &ign);
					if(res < 0) { goto FAILED; }
					else if(res > 0) { break; }
					assert(ign == sizeof(PTask **));
					PWhen *pwhen = (*tracker.task)->when(tracker.task);
					usize count = 0;
					while(pwhen->when) {
						struct pollfd temp;
						temp.fd = pwhen->fd;
						switch(pwhen->when) {
							case PWHEN_IN: { temp.events = POLLIN; } break;
							case PWHEN_OUT: { temp.events = POLLOUT; } break;
							default: { temp.events = POLLIN | POLLOUT; } break;
						}
						++count;
						++pwhen;

						if(PollFDsPush(&poller->fds, temp)) {
							DEBUG_LOG("PollFDsPush(&poller->fds, temp) has failed\n");
							(*tracker.task)->de(tracker.task);
							goto FAILED;
						}
					}
					tracker.len = count;
					if(PollTaskTrackersPush(&poller->trackers, tracker)) {
						DEBUG_LOG("PollTaskTrackersPush(&poller->trackers, tracker) has failed\n");
						(*tracker.task)->de(tracker.task);
						goto FAILED;
					}
				}
			}
			++fd;

			PollTaskTracker *tracker = PollTaskTrackersData(&poller->trackers);
			PollTaskTracker *trackerEnd = tracker + PollTaskTrackersSize(&poller->trackers);
			
			while(tracker != trackerEnd) {
				for(struct pollfd *iter = fd, *fdEnd = fd + tracker->len; iter != fdEnd; ++iter) {
					if(iter->events & iter->revents) {
						Ret res;
						NEG_CHECK(res = (*tracker->task)->cb(tracker->task));
						if(PTASK_RET_DONE == res) { (*tracker->task)->de(tracker->task); tracker->task = NULL; }
						else if(PTASK_RET_SPARE == res) { tracker->task = NULL; }
						break;
					}
				}
				if(NULL == tracker->task) {
					for(struct pollfd *iter = fd, *fdEnd = fd + tracker->len; iter != fdEnd; ++iter) {
						iter->events = 0;
					}
				}


				fd += tracker->len;
				++tracker;
			}

			PollFDsRm(&poller->fds, 1, PollFDsSize(&poller->fds), &POLL_CLEAR_PRED2);
			PollTaskTrackersRm(&poller->trackers, 0, PollTaskTrackersSize(&poller->trackers), &POLL_CLEAR_PRED);
		}
		else {
			PollFDsSetSize(&poller->fds, 1);
			PollTaskTracker *tracker = PollTaskTrackersData(&poller->trackers);
			PollTaskTracker *trackerEnd = tracker + PollTaskTrackersSize(&poller->trackers);
			while(tracker != trackerEnd) {
				Ret res;
				NEG_CHECK(res = (*tracker->task)->cb(tracker->task));
				if(PTASK_RET_DONE == res) { (*tracker->task)->de(tracker->task); tracker->task = NULL; }
				else if(PTASK_RET_SPARE == res) { tracker->task = NULL; }
				++tracker;
			}

			PollTaskTrackersRm(&poller->trackers, 0, PollTaskTrackersSize(&poller->trackers), &POLL_CLEAR_PRED);
		}
	}

FAILED:
	PipeCloseWriteEnd(&poller->pip);
	return;
}
