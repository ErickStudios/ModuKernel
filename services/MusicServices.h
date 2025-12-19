#ifndef MusicServicesDotH
#define MusicServicesDotH
/* el tipo de setear tono */
typedef void (*KernelSimpleMusicSetTone)(unsigned int Tone);
/* el tipo de parar tono */
typedef void (*KernelSimpleMusicMuteTone)();
/* la estructura */
typedef struct _MusicServices {
    /* funcion para ajustar el tono con su frecuencia al beeper
    de la pc */
    KernelSimpleMusicSetTone PlayTone;
    /* funcion para mutear el beeper*/
    KernelSimpleMusicMuteTone Mute;
} MusicServices;
#endif