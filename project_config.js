//██▀▀█▀▀██▀▀▀▀▀▀▀███▀▀█▀▀▀▀▀▀▀▀█
//██  ▀  █▄  ▄▄▄▄  ▀  ▄█ ▄▀▀ █  █
//█  ▄ ▄  ▀▀  ▀█▀  ▄  ▀█ ▀▄█ █▄ █
//█▄▄█▄█▄▄▄▄▄▄██▄▄███▄▄█▄▄▄▄▄▄▄▄█
// by Guillaume 'Aoineko' Blanchard under CC BY-SA license

//*****************************************************************************
// TOOLS SETTINGS
//*****************************************************************************

Emulator = `${ToolsDir}OpenMSX/openmsx`;
// Emulator = `${ToolsDir}Emulicious/Emulicious`;
// Emulator = `${ToolsDir}BlueMSX/blueMSX`;
// Emulator = `${ToolsDir}MEISEI/meisei`;
// Emulator = `${ToolsDir}fMSX/fMSX`;
// Emulator = `${ToolsDir}RuMSX/MSX`;

//*****************************************************************************
// PROJECT SETTINGS
//*****************************************************************************

// Project name (will be use for output filename)
ProjName = "tennis";

// Project modules to build (use ProjName if not defined)
ProjModules = [ ProjName ];

// List of library modules to build
LibModules = [ "vdp", "print", "input", "memory", "math", "game", "pt3/pt3_player", "ayfx/ayfx_player" ];

// MSX version:
// - 1		MSX 1
// - 2		MSX 2
// - 2P		MSX 2+
// - TR		MSX TurboR
// - 12		MSX 1/2
Machine = "12";

// Target:
// - BIN			.bin	BASIC binary program (8000h~)
// - ROM_8K			.rom	8KB ROM in page 1 (4000h ~ 5FFFh)
// - ROM_8K_P2		.rom	8KB ROM in page 2 (8000h ~ 9FFFh)
// - ROM_16K		.rom	16KB ROM in page 1 (4000h ~ 7FFFh)
// - ROM_16K_P2		.rom	16KB ROM in page 2 (8000h ~ BFFFh)
// - ROM_32K		.rom	32KB ROM in page 1-2 (4000h ~ BFFFh)
// - ROM_48K		.rom	48KB ROM in page 0-2 (0000h ~ BFFFh). Pages 1-2 visible at start
// - ROM_48K_ISR	.rom	48KB ROM in page 0-2 (0000h ~ BFFFh). Pages 0-2 visible at start
// - ROM_64K		.rom	64KB ROM in page 0-3 (0000h ~ FFFFh). Pages 1-2 visible at start
// - ROM_64K_ISR	.rom	64KB ROM in page 0-3 (0000h ~ FFFFh). Pages 0-2 visible at start
// - ROM_ASCII8		.rom	128KB ROM using ASCII-8 mapper
// - ROM_ASCII16	.rom	128KB ROM using ASCII-16 mapper
// - ROM_KONAMI		.rom	128KB ROM using Konami mapper (8KB segments)
// - ROM_KONAMI_SCC	.rom	128KB ROM using Konami SCC mapper (8KB segments)
// - DOS1			.com	MSX-DOS 1 program (0100h~) No direct acces to Main-ROM
// - DOS2			.com	MSX-DOS 2 program (0100h~) No direct acces to Main-ROM
Target = "ROM_48K_ISR";

// ROM mapper size (from 64 to 4096). Must be a multiple of 8 or 16 depending on the mapper type
ROMSize = 0;

// Postpone the ROM startup to let the other ROMs initialize (BDOS for example) (0 = false, 1 = true)
ROMDelayBoot = false;

// Overwrite RAM starting address
ForceRamAddr = 0xE000;

// Add application signature to binary data (0 = false, 1 = true)
AppSignature = true;

// Application company
AppCompany = "PP";

// Application ID (0~65535)
AppID = "FM";

//*******************************************************************************
// MAKE SETTINGS
//*******************************************************************************

// debug flag (0 = false, 1 = true)
Debug = true;

// Optim:
// - Default
// - Speed
// - Size
Optim = "Speed";

// Additionnal compilation flag
CompileOpt = "";

// Skip file if compile data is newer than the source (0 = false, 1 = true)
CompileSkipOld = false;

// Verbose mode (0 = false, 1 = true)
Verbose = false;

//*******************************************************************************
// EMULATOR SETINGS
//*******************************************************************************

// Emulator options (0 = false, 1 = true)
EmulMachine = false;
Emul60Hz = false;
EmulFullScreen = false;
EmulMute = false;
EmulDebug = false;
EmulSCC = false;
EmulMSXMusic = false;
EmulMSXAudio = false;

// Emulator extra parameters to be add to command-line
EmulExtraParam = "";

//*****************************************************************************
// BUILD STEPS
//*****************************************************************************

DoClean   = false;
DoCompile = true;
DoMake    = true;
DoPackage = true;
DoDeploy  = true;
DoRun     = true;
