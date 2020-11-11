mount -t tmpfs tmpfs disk -o size=1024M
df -h disk
cd disk
touch log && chown user log
touch typing && chown user typing
touch present && chown user present
touch alarm && chown user alarm
touch minput && chown user minput
touch finput && chown user finput
touch iinput && chown user iinput
touch tinput && chown user tinput
touch tinput && chown user tinput
#mount -t tmpfs tmpfs /mtn/tmp -o size=1024M
#mount -t ramfs ramfs /mtn/ram -o size=1024M // can do size for ramfs now
#umount /mtn/tmp
