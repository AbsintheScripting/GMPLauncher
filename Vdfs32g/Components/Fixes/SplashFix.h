#ifndef _SPLASH_FIX_
#define _SPLASH_FIX_

// SplashFix: intercept LoadBitmapA to replace the game's splash screen with a custom one from VDF
extern bool InstallSplashFix();
// RemoveSplashFix: no cleanup needed for the splash fix
extern void RemoveSplashFix();

#endif
