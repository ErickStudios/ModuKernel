cd ..

echo Bienvenido a ModuKernel ahorita mismo se activo el modo sin compilar

# arranca qemu
qemu-system-i386 \
  -cdrom build/os.iso \
  -boot d \
  -hda build/disk.img \
  -m 512M \
  -vga std \
  -audiodev pa,id=snd0,out.frequency=44100,out.channels=2
