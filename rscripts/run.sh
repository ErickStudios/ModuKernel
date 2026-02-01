cd ..

echo Bienvenido a ModuKernel ahorita mismo se activo el modo sin compilar

# arranca qemu
qemu-system-i386                                              \
  -cpu 486,+x87                                               \
  -cdrom build/os.iso                                         \
  -boot d                                                     \
  -fda build/floppy.img                                       \
  -hda build/disk.img                                         \
  -m 256M                                                     \
  -vga std                                                    \
    -device isa-debugcon,chardev=mimami                       \
    -chardev stdio,id=mimami                                  \
  -audiodev pa,id=snd0,out.frequency=44100,out.channels=2
