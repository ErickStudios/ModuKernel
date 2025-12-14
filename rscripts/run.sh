cd ..

echo Bienvenido a ModuKernel ahorita mismo se activo el modo sin compilar

# arranca qemu
qemu-system-i386 -kernel ./build/kernel -hda ./build/disk.img -m 512M