#ifndef _KILLER_FIX_
#define _KILLER_FIX_

// KillerFix: applies game-specific memory patches loaded from .patch files to fix bugs/performance
extern bool InstallKillerFix();
// RemoveKillerFix: frees memory blocks allocated during patch installation
extern void RemoveKillerFix();

#endif
