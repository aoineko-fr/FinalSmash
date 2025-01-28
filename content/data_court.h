// ██▀▀█▀▀██▀▀▀▀▀▀▀█▀▀█ ▄
// ██  ▀  █▄  ▀██▄ ▀ ▄█ ▄  ▄█▄█ ▄▀██
// █  █ █  ▀▀  ▄█  █  █ ██ ██ █  ▀██
// ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀          ▀▀
// Data generated using MSXimg 1.17.0 on Sun Mar 24 11:59:55 2024
// by Guillaume "Aoineko" Blanchard (2022) under CC BY-SA free license
// ─────────────────────────────────────────────────────────────────────────────
// Generation parameters:
//  - Input file:     datasrc\court.png
//  - Mode:           Graphic Mode 2
//  - Start position: 24, 24
//  - Sprite size:    216, 144 (gap: 0, 0)
//  - Sprite count:   1, 1
//  - Color count:    256 (Transparent: #0000)
//  - Compressor:     RLEp (6-bits Pattern based RLE)
//  - Skip empty:     FALSE
//  - Offset:         0

// Names Table
const unsigned char g_DataCourt_Names[] =
{
	0x00, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x00, 0x00, 
	0x00, 0x04, 0x05, 0x05, 0x06, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x07, 0x08, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x09, 0x05, 0x05, 0x0A, 0x00, 0x00, 
	0x00, 0x0B, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x00, 0x00, 0x0E, 0x00, 0x00, 
	0x00, 0x0C, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x00, 0x00, 0x0D, 0x00, 0x00, 
	0x00, 0x0C, 0x00, 0x00, 0x0F, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x10, 0x00, 0x00, 0x0D, 0x00, 0x00, 
	0x00, 0x0C, 0x00, 0x11, 0x12, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x13, 0x12, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x13, 0x14, 0x00, 0x0D, 0x00, 0x00, 
	0x00, 0x0C, 0x00, 0x11, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x14, 0x00, 0x0D, 0x00, 0x00, 
	0x15, 0x16, 0x17, 0x18, 0x19, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x18, 0x19, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x18, 0x19, 0x17, 0x1A, 0x1B, 0x00, 
	0x0D, 0x1C, 0x1D, 0x1E, 0x1F, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1E, 0x1F, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1E, 0x1F, 0x1D, 0x20, 0x21, 0x00, 
	0x22, 0x23, 0x24, 0x25, 0x26, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x25, 0x26, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x25, 0x26, 0x24, 0x27, 0x23, 0x28, 
	0x11, 0x14, 0x00, 0x11, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x14, 0x00, 0x11, 0x14, 0x00, 
	0x0D, 0x00, 0x00, 0x11, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x14, 0x00, 0x00, 0x0C, 0x00, 
	0x0D, 0x00, 0x00, 0x11, 0x29, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x2A, 0x29, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x2A, 0x14, 0x00, 0x00, 0x0C, 0x00, 
	0x0D, 0x00, 0x00, 0x0D, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x0C, 0x00, 0x00, 0x0C, 0x00, 
	0x0D, 0x00, 0x00, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x0C, 0x00, 
	0x0E, 0x00, 0x00, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x0B, 0x00, 
	0x2B, 0x02, 0x02, 0x10, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x2C, 0x2D, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0F, 0x02, 0x02, 0x2E, 0x00, 
	0x2F, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x30, 0x00, 
};
// Names size: 486 Bytes

// Patterns Table
const unsigned char g_DataCourt_Patterns[] =
{
// Chunk[0]
	0x0F, // Type=0, Length=15
// Chunk[1]
	0xC1, // Type=3, Length=1
	0x7F, /* .####### */ 
// Chunk[2]
	0x0F, // Type=0, Length=15
// Chunk[3]
	0xC2, // Type=3, Length=2
	0xFE, /* #######. */ 
	0x7F, /* .####### */ 
// Chunk[4]
	0x47, // Type=1, Length=7
	0x60, /* .##..... */ 
// Chunk[5]
	0x09, // Type=0, Length=9
// Chunk[6]
	0x47, // Type=1, Length=7
	0xC0, /* ##...... */ 
// Chunk[7]
	0xC1, // Type=3, Length=1
	0x00, /* ........ */ 
// Chunk[8]
	0x43, // Type=1, Length=3
	0x01, /* .......# */ 
// Chunk[9]
	0x05, // Type=0, Length=5
// Chunk[10]
	0x43, // Type=1, Length=3
	0x80, /* #....... */ 
// Chunk[11]
	0x05, // Type=0, Length=5
// Chunk[12]
	0x47, // Type=1, Length=7
	0x03, /* ......## */ 
// Chunk[13]
	0xC1, // Type=3, Length=1
	0xFE, /* #######. */ 
// Chunk[14]
	0x47, // Type=1, Length=7
	0x06, /* .....##. */ 
// Chunk[15]
	0x48, // Type=1, Length=8
	0x60, /* .##..... */ 
// Chunk[16]
	0x48, // Type=1, Length=8
	0xC0, /* ##...... */ 
// Chunk[17]
	0x48, // Type=1, Length=8
	0x03, /* ......## */ 
// Chunk[18]
	0x48, // Type=1, Length=8
	0x06, /* .....##. */ 
// Chunk[19]
	0x47, // Type=1, Length=7
	0xC0, /* ##...... */ 
// Chunk[20]
	0xC1, // Type=3, Length=1
	0x00, /* ........ */ 
// Chunk[21]
	0x47, // Type=1, Length=7
	0x03, /* ......## */ 
// Chunk[22]
	0xC1, // Type=3, Length=1
	0x00, /* ........ */ 
// Chunk[23]
	0x48, // Type=1, Length=8
	0x01, /* .......# */ 
// Chunk[24]
	0xC1, // Type=3, Length=1
	0x00, /* ........ */ 
// Chunk[25]
	0x47, // Type=1, Length=7
	0x80, /* #....... */ 
// Chunk[26]
	0xC1, // Type=3, Length=1
	0x00, /* ........ */ 
// Chunk[27]
	0x47, // Type=1, Length=7
	0x01, /* .......# */ 
// Chunk[28]
	0x48, // Type=1, Length=8
	0x80, /* #....... */ 
// Chunk[29]
	0x42, // Type=1, Length=2
	0x01, /* .......# */ 
// Chunk[30]
	0x46, // Type=1, Length=6
	0x03, /* ......## */ 
// Chunk[31]
	0x42, // Type=1, Length=2
	0x80, /* #....... */ 
// Chunk[32]
	0xC6, // Type=3, Length=6
	0x3F, /* ..###### */ 
	0x03, /* ......## */ 
	0xD5, /* ##.#.#.# */ 
	0xEA, /* ###.#.#. */ 
	0xD5, /* ##.#.#.# */ 
	0xEA, /* ###.#.#. */ 
// Chunk[33]
	0x04, // Type=0, Length=4
// Chunk[34]
	0xC4, // Type=3, Length=4
	0x55, /* .#.#.#.# */ 
	0xAA, /* #.#.#.#. */ 
	0x55, /* .#.#.#.# */ 
	0xAA, /* #.#.#.#. */ 
// Chunk[35]
	0x42, // Type=1, Length=2
	0x01, /* .......# */ 
// Chunk[36]
	0x02, // Type=0, Length=2
// Chunk[37]
	0xC4, // Type=3, Length=4
	0x55, /* .#.#.#.# */ 
	0xAB, /* #.#.#.## */ 
	0x55, /* .#.#.#.# */ 
	0xAB, /* #.#.#.## */ 
// Chunk[38]
	0x42, // Type=1, Length=2
	0x80, /* #....... */ 
// Chunk[39]
	0x02, // Type=0, Length=2
// Chunk[40]
	0xC4, // Type=3, Length=4
	0xD5, /* ##.#.#.# */ 
	0xAA, /* #.#.#.#. */ 
	0xD5, /* ##.#.#.# */ 
	0xAA, /* #.#.#.#. */ 
// Chunk[41]
	0x42, // Type=1, Length=2
	0x01, /* .......# */ 
// Chunk[42]
	0xC6, // Type=3, Length=6
	0xFE, /* #######. */ 
	0x00, /* ........ */ 
	0x57, /* .#.#.### */ 
	0xAB, /* #.#.#.## */ 
	0x57, /* .#.#.### */ 
	0xAB, /* #.#.#.## */ 
// Chunk[43]
	0x42, // Type=1, Length=2
	0x80, /* #....... */ 
// Chunk[44]
	0x46, // Type=1, Length=6
	0xC0, /* ##...... */ 
// Chunk[45]
	0xE8, // Type=3, Length=40
	0xD5, /* ##.#.#.# */ 
	0xEA, /* ###.#.#. */ 
	0xD5, /* ##.#.#.# */ 
	0xEA, /* ###.#.#. */ 
	0xD5, /* ##.#.#.# */ 
	0xEA, /* ###.#.#. */ 
	0xD5, /* ##.#.#.# */ 
	0x00, /* ........ */ 
	0x55, /* .#.#.#.# */ 
	0xAA, /* #.#.#.#. */ 
	0x55, /* .#.#.#.# */ 
	0xAA, /* #.#.#.#. */ 
	0x55, /* .#.#.#.# */ 
	0xAA, /* #.#.#.#. */ 
	0x55, /* .#.#.#.# */ 
	0xAA, /* #.#.#.#. */ 
	0x55, /* .#.#.#.# */ 
	0xAB, /* #.#.#.## */ 
	0x55, /* .#.#.#.# */ 
	0xAB, /* #.#.#.## */ 
	0x55, /* .#.#.#.# */ 
	0xAB, /* #.#.#.## */ 
	0x55, /* .#.#.#.# */ 
	0xAA, /* #.#.#.#. */ 
	0xD5, /* ##.#.#.# */ 
	0xAA, /* #.#.#.#. */ 
	0xD5, /* ##.#.#.# */ 
	0xAA, /* #.#.#.#. */ 
	0xD5, /* ##.#.#.# */ 
	0xAA, /* #.#.#.#. */ 
	0xD5, /* ##.#.#.# */ 
	0xAA, /* #.#.#.#. */ 
	0x57, /* .#.#.### */ 
	0xAB, /* #.#.#.## */ 
	0x57, /* .#.#.### */ 
	0xAB, /* #.#.#.## */ 
	0x57, /* .#.#.### */ 
	0xAB, /* #.#.#.## */ 
	0x57, /* .#.#.### */ 
	0xAB, /* #.#.#.## */ 
// Chunk[46]
	0x48, // Type=1, Length=8
	0xC0, /* ##...... */ 
// Chunk[47]
	0xC2, // Type=3, Length=2
	0x01, /* .......# */ 
	0xFE, /* #######. */ 
// Chunk[48]
	0x46, // Type=1, Length=6
	0x01, /* .......# */ 
// Chunk[49]
	0xC2, // Type=3, Length=2
	0x80, /* #....... */ 
	0x00, /* ........ */ 
// Chunk[50]
	0x46, // Type=1, Length=6
	0x80, /* #....... */ 
// Chunk[51]
	0x09, // Type=0, Length=9
// Chunk[52]
	0x47, // Type=1, Length=7
	0x01, /* .......# */ 
// Chunk[53]
	0xC1, // Type=3, Length=1
	0x00, /* ........ */ 
// Chunk[54]
	0x47, // Type=1, Length=7
	0x80, /* #....... */ 
// Chunk[55]
	0xC2, // Type=3, Length=2
	0x01, /* .......# */ 
	0xFE, /* #######. */ 
// Chunk[56]
	0x46, // Type=1, Length=6
	0x01, /* .......# */ 
// Chunk[57]
	0xC1, // Type=3, Length=1
	0x7F, /* .####### */ 
// Chunk[58]
	0x07, // Type=0, Length=7
// Chunk[59]
	0x47, // Type=1, Length=7
	0x80, /* #....... */ 
// Chunk[60]
	0xC1, // Type=3, Length=1
	0x00, /* ........ */ 
// Chunk[61]
	0x47, // Type=1, Length=7
	0x01, /* .......# */ 
// Chunk[62]
	0xC1, // Type=3, Length=1
	0x00, /* ........ */ 
// Chunk[63]
	0x47, // Type=1, Length=7
	0x06, /* .....##. */ 
// Chunk[64]
	0xC1, // Type=3, Length=1
	0x07, /* .....### */ 
// Chunk[65]
	0x04, // Type=0, Length=4
// Chunk[66]
	0x43, // Type=1, Length=3
	0x01, /* .......# */ 
// Chunk[67]
	0x05, // Type=0, Length=5
// Chunk[68]
	0x43, // Type=1, Length=3
	0x80, /* #....... */ 
// Chunk[69]
	0xC1, // Type=3, Length=1
	0x00, /* ........ */ 
// Chunk[70]
	0x47, // Type=1, Length=7
	0x60, /* .##..... */ 
// Chunk[71]
	0xC2, // Type=3, Length=2
	0xE0, /* ###..... */ 
	0x07, /* .....### */ 
// Chunk[72]
	0x07, // Type=0, Length=7
// Chunk[73]
	0xC1, // Type=3, Length=1
	0xE0, /* ###..... */ 
// Chunk[74]
	0x07, // Type=0, Length=7
// Zero terminator
	0x00, // 
};
// Patterns size: 200 Bytes

// Colors Table
const unsigned char g_DataCourt_Colors[] =
{
// Chunk[0]
	0x4F, // Type=1, Length=15
	0x99, /* #..##..# */ 
// Chunk[1]
	0xC1, // Type=3, Length=1
	0xF9, /* #####..# */ 
// Chunk[2]
	0x47, // Type=1, Length=7
	0x99, /* #..##..# */ 
// Chunk[3]
	0xC1, // Type=3, Length=1
	0xFF, /* ######## */ 
// Chunk[4]
	0x47, // Type=1, Length=7
	0x99, /* #..##..# */ 
// Chunk[5]
	0x49, // Type=1, Length=9
	0xF9, /* #####..# */ 
// Chunk[6]
	0xC1, // Type=3, Length=1
	0xFF, /* ######## */ 
// Chunk[7]
	0x47, // Type=1, Length=7
	0x99, /* #..##..# */ 
// Chunk[8]
	0xC1, // Type=3, Length=1
	0xFF, /* ######## */ 
// Chunk[9]
	0x47, // Type=1, Length=7
	0xF9, /* #####..# */ 
// Chunk[10]
	0xC1, // Type=3, Length=1
	0xFF, /* ######## */ 
// Chunk[11]
	0x43, // Type=1, Length=3
	0xF9, /* #####..# */ 
// Chunk[12]
	0x44, // Type=1, Length=4
	0x99, /* #..##..# */ 
// Chunk[13]
	0xC1, // Type=3, Length=1
	0xFF, /* ######## */ 
// Chunk[14]
	0x43, // Type=1, Length=3
	0xF9, /* #####..# */ 
// Chunk[15]
	0x44, // Type=1, Length=4
	0x99, /* #..##..# */ 
// Chunk[16]
	0xC1, // Type=3, Length=1
	0xFF, /* ######## */ 
// Chunk[17]
	0x76, // Type=1, Length=54
	0xF9, /* #####..# */ 
// Chunk[18]
	0xC1, // Type=3, Length=1
	0xFF, /* ######## */ 
// Chunk[19]
	0x47, // Type=1, Length=7
	0xF9, /* #####..# */ 
// Chunk[20]
	0xC1, // Type=3, Length=1
	0xFF, /* ######## */ 
// Chunk[21]
	0x48, // Type=1, Length=8
	0xF9, /* #####..# */ 
// Chunk[22]
	0xC1, // Type=3, Length=1
	0xFF, /* ######## */ 
// Chunk[23]
	0x47, // Type=1, Length=7
	0xF9, /* #####..# */ 
// Chunk[24]
	0xC1, // Type=3, Length=1
	0xFF, /* ######## */ 
// Chunk[25]
	0x50, // Type=1, Length=16
	0xF9, /* #####..# */ 
// Chunk[26]
	0xC1, // Type=3, Length=1
	0xE9, /* ###.#..# */ 
// Chunk[27]
	0x47, // Type=1, Length=7
	0xF9, /* #####..# */ 
// Chunk[28]
	0xC1, // Type=3, Length=1
	0xE9, /* ###.#..# */ 
// Chunk[29]
	0x42, // Type=1, Length=2
	0xFE, /* #######. */ 
// Chunk[30]
	0x44, // Type=1, Length=4
	0xE9, /* ###.#..# */ 
// Chunk[31]
	0x42, // Type=1, Length=2
	0x99, /* #..##..# */ 
// Chunk[32]
	0x42, // Type=1, Length=2
	0xFF, /* ######## */ 
// Chunk[33]
	0x46, // Type=1, Length=6
	0xF9, /* #####..# */ 
// Chunk[34]
	0x42, // Type=1, Length=2
	0xFF, /* ######## */ 
// Chunk[35]
	0x46, // Type=1, Length=6
	0xF9, /* #####..# */ 
// Chunk[36]
	0x42, // Type=1, Length=2
	0xFF, /* ######## */ 
// Chunk[37]
	0x45, // Type=1, Length=5
	0xF9, /* #####..# */ 
// Chunk[38]
	0xC3, // Type=3, Length=3
	0xE9, /* ###.#..# */ 
	0xFE, /* #######. */ 
	0xFF, /* ######## */ 
// Chunk[39]
	0x45, // Type=1, Length=5
	0xF9, /* #####..# */ 
// Chunk[40]
	0x4E, // Type=1, Length=14
	0xE9, /* ###.#..# */ 
// Chunk[41]
	0xC1, // Type=3, Length=1
	0xEE, /* ###.###. */ 
// Chunk[42]
	0x47, // Type=1, Length=7
	0xF9, /* #####..# */ 
// Chunk[43]
	0xC1, // Type=3, Length=1
	0xFE, /* #######. */ 
// Chunk[44]
	0x47, // Type=1, Length=7
	0xF9, /* #####..# */ 
// Chunk[45]
	0xC1, // Type=3, Length=1
	0xFE, /* #######. */ 
// Chunk[46]
	0x47, // Type=1, Length=7
	0xF9, /* #####..# */ 
// Chunk[47]
	0xC1, // Type=3, Length=1
	0xFE, /* #######. */ 
// Chunk[48]
	0x47, // Type=1, Length=7
	0xF9, /* #####..# */ 
// Chunk[49]
	0xC1, // Type=3, Length=1
	0xFE, /* #######. */ 
// Chunk[50]
	0x47, // Type=1, Length=7
	0xE9, /* ###.#..# */ 
// Chunk[51]
	0xC3, // Type=3, Length=3
	0xE1, /* ###....# */ 
	0xF9, /* #####..# */ 
	0x91, /* #..#...# */ 
// Chunk[52]
	0x46, // Type=1, Length=6
	0xF9, /* #####..# */ 
// Chunk[53]
	0xC2, // Type=3, Length=2
	0xE1, /* ###....# */ 
	0x11, /* ...#...# */ 
// Chunk[54]
	0x46, // Type=1, Length=6
	0xF9, /* #####..# */ 
// Chunk[55]
	0xC1, // Type=3, Length=1
	0x11, /* ...#...# */ 
// Chunk[56]
	0x47, // Type=1, Length=7
	0x99, /* #..##..# */ 
// Chunk[57]
	0xC1, // Type=3, Length=1
	0x11, /* ...#...# */ 
// Chunk[58]
	0x47, // Type=1, Length=7
	0xF9, /* #####..# */ 
// Chunk[59]
	0xC1, // Type=3, Length=1
	0x11, /* ...#...# */ 
// Chunk[60]
	0x47, // Type=1, Length=7
	0xF9, /* #####..# */ 
// Chunk[61]
	0xC2, // Type=3, Length=2
	0xF1, /* ####...# */ 
	0x91, /* #..#...# */ 
// Chunk[62]
	0x46, // Type=1, Length=6
	0xF9, /* #####..# */ 
// Chunk[63]
	0xC1, // Type=3, Length=1
	0x91, /* #..#...# */ 
// Chunk[64]
	0x47, // Type=1, Length=7
	0x99, /* #..##..# */ 
// Chunk[65]
	0x47, // Type=1, Length=7
	0xF9, /* #####..# */ 
// Chunk[66]
	0xC1, // Type=3, Length=1
	0xFF, /* ######## */ 
// Chunk[67]
	0x47, // Type=1, Length=7
	0xF9, /* #####..# */ 
// Chunk[68]
	0xC1, // Type=3, Length=1
	0xFF, /* ######## */ 
// Chunk[69]
	0x48, // Type=1, Length=8
	0xF9, /* #####..# */ 
// Chunk[70]
	0x44, // Type=1, Length=4
	0x99, /* #..##..# */ 
// Chunk[71]
	0x43, // Type=1, Length=3
	0xF9, /* #####..# */ 
// Chunk[72]
	0xC1, // Type=3, Length=1
	0xFF, /* ######## */ 
// Chunk[73]
	0x44, // Type=1, Length=4
	0x99, /* #..##..# */ 
// Chunk[74]
	0x43, // Type=1, Length=3
	0xF9, /* #####..# */ 
// Chunk[75]
	0xC1, // Type=3, Length=1
	0xFF, /* ######## */ 
// Chunk[76]
	0x49, // Type=1, Length=9
	0xF9, /* #####..# */ 
// Chunk[77]
	0x47, // Type=1, Length=7
	0x99, /* #..##..# */ 
// Chunk[78]
	0xC1, // Type=3, Length=1
	0xF9, /* #####..# */ 
// Chunk[79]
	0x47, // Type=1, Length=7
	0x99, /* #..##..# */ 
// Zero terminator
	0x00, // 
};
// Colors size: 167 Bytes

// Total size: 853 Bytes
