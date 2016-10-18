#define WAVPACK_MAGIC	"wavpack1.0"

#define RUNL 256		/* max run length <=256 */
				/* compression format flags */
#define BYTE 0			/* 8 bit data format */
#define SHORT 0x20		/* 16 bit data format (else 8 bit) */
#define REV 0x10		/* byte reversed (SHORT only) */
#define OFFSET 0x8		/* 1 byte data offset */
#define DIFF 0x4		/* differential pcm */
#define STEREO 0x2		/* 2 channel */
#define CHAN1 0x1		/* channel nr (0 or 1) */

#define WAV 1			/* modes */
#define UNWAV 2
#define WCAT 3
