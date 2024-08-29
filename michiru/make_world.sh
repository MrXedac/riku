MODULES="init hello ls lsdev"
CURDIR=`pwd`
for mod in $MODULES
do
    cd $mod && make
    cd $CURDIR
    cp $mod/$mod initramfs/$mod
done

cd initramfs
tar --format ustar -cvf initramfs.img *
mv initramfs.img ../../boot/boot/
cd $CURDIR
