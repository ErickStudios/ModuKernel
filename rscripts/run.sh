cd ..

echo Bienvenido a ModuKernel ahorita mismo se activo el modo sin compilar

# arranca qemu
qemu-system-i386                                              \
  -cpu pentium3                                               \
  -cdrom build/os.iso                                         \
  -boot d                                                     \
  -fda build/floppy.img                                       \
  -hda build/disk.img                                         \
  -m 256M                                                     \
  -vga std                                                    \
  -device isa-debugcon,chardev=mimami                         \
  -chardev stdio,id=mimami                                    \
  -netdev user,id=net0                                        \
  -device rtl8139,netdev=net0,mac=52:54:00:12:34:56           \
  -audiodev pa,id=snd0,out.frequency=44100,out.channels=2