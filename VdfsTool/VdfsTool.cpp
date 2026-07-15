// VdfsTool.cpp: main project file.

#include "stdafx.h"
#include "Form1.h"

using namespace VdfsTool;

[STAThreadAttribute]
int main(array<System::String ^> ^args)
{
	// Enabling Windows XP visual effects before creating any controls
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false); 

	// Creation of the main window and its launch
	Application::Run(gcnew Form1());
	return 0;
}
