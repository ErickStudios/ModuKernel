/* piano */
#include "../library/lib.h"

typedef enum _MusicalNotes
{
    NoteDo = 261,
    NoteRe = 277,
    NoteMi = 329,
    NoteFa = 349,
    NoteSol = 415,
    NoteLa = 440,
    NoteSi = 493
} MusicalNotes;

KernelStatus LoopMakeSong(KernelServices* Services)
{
    while (1)
    {
        Services->Display->printg("> ");
        char* Line = Services->InputOutput->ReadLine();
        Services->Display->printg("\n");

        if (StrCmp(Line, "exit") == 0) return KernelStatusSuccess;
        else Services->Music->PlayTone(StringToInt(Line));

        Services->Memory->FreePool(Line);
    }
}

KernelStatus ErickMain(KernelServices* Services)
{
    while (1)
    {
        char Key = Services->InputOutput->WaitKey();

        if (Key == '1') Services->Music->PlayTone(NoteDo);
        else if (Key == '2') Services->Music->PlayTone(NoteRe);
        else if (Key == '3') Services->Music->PlayTone(NoteMi);
        else if (Key == '4') Services->Music->PlayTone(NoteFa);
        else if (Key == '5') Services->Music->PlayTone(NoteSol);
        else if (Key == '6') Services->Music->PlayTone(NoteLa);
        else if (Key == '7') Services->Music->PlayTone(NoteSi);
        else if (Key == '8') Services->Music->PlayTone(NoteDo);
        else if (Key == '\n') {
            Services->Music->Mute();
            return KernelStatusSuccess;
        }
        else if (Key == 't' || Key == 'T') LoopMakeSong(Services);
        else Services->Music->Mute();
    }
}