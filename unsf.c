/*
 * unsf breaks up sound fonts into GUS type patch files.
 *
 * usage: unsf <name of sound font file>
 *
 * compile: cc -o unsf unsf.c -lm
 *
 * Credits:
 * unsf.c is derived from the Allegro tool pat2dat.c, and
 * uses, presumably, the part of that program which was written
 * originally by George Foot.  The original file intro of pat2dat.c
 * and information on copyright is given below.  Also, some code
 * is adapted from routines in sndfont.c, whose header has
 * the notice:
 *================================================================
 * SoundFont file extension
 *      written by Takashi Iwai <iwai@dragon.mm.t.u-tokyo.ac.jp>
 *================================================================*
 *
 *
 * 						Greg Lee, 6/25/04
 */


/*         ______   ___    ___
 *        /\  _  \ /\_ \  /\_ \ 
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      MIDI patch (for DIGMID) grabbing utility for the Allegro library.
 *
 *      By Shawn Hargreaves.
 *
 *      SoundFont loader based on code provided by George Foot.
 *
 *      See readme.txt for copyright information.
 */

/*
 * (The above mentioned file readme.txt has the following about copyright:)
===================================
============ Copyright ============
===================================

   Allegro is gift-ware. It was created by a number of people working in 
   cooperation, and is given to you freely as a gift. You may use, modify, 
   redistribute, and generally hack it about in any way you like, and you do 
   not have to give us anything in return. However, if you like this product 
   you are encouraged to thank us by making a return gift to the Allegro 
   community. This could be by writing an add-on package, providing a useful 
   bug report, making an improvement to the library, or perhaps just 
   releasing the sources of your program so that other people can learn from 
   them. If you redistribute parts of this code or make a game using it, it 
   would be nice if you mentioned Allegro somewhere in the credits, but you 
   are not required to do this. We trust you not to abuse our generosity.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifndef TRUE 
   #define TRUE         -1
   #define FALSE        0
#endif

#undef MIN
#undef MAX
#undef MID
     
#define MIN(x,y)     (((x) < (y)) ? (x) : (y))
#define MAX(x,y)     (((x) > (y)) ? (x) : (y))
#define MID(x,y,z)   MAX((x), MIN((y), (z)))
     
#define LINKED_SAMPLE 8
#define LEFT_SAMPLE 4
#define RIGHT_SAMPLE 2
#define MONO_SAMPLE 1

static int err = 0;


typedef struct VelocityRangeList
{
   int range_count;
   unsigned char velmin[128];
   unsigned char velmax[128];
   unsigned char mono_patches[128];
   unsigned char left_patches[128];
   unsigned char right_patches[128];
   unsigned char other_patches[128];
} VelocityRangeList;

static int opt_8bit = FALSE;
static int opt_verbose = FALSE;
static int opt_veryverbose = FALSE;
static int opt_bank = 0;
static int opt_drum_bank = 0;
static int opt_header = 0;
static int opt_left_channel = 0;
static int opt_right_channel = 0;
static int wanted_velmin, wanted_velmax;
static char *opt_soundfont = NULL;
static int opt_small = FALSE;
static int opt_drum = FALSE;
static int opt_mono = FALSE;
static int opt_no_write = 0;
static int opt_adjust_sample_flags = 0;
static int opt_adjust_volume = 1;

static int tonebank[128];
static char *tonebank_name[128];
static char *voice_name[128][128];
static int voice_samples_mono[128][128];
static int voice_samples_left[128][128];
static int voice_samples_right[128][128];
static VelocityRangeList *voice_velocity[128][128];
static char *drumset_name[128];
static char *drumset_short_name[128];
static char *drum_name[128][128];
static int drum_samples_mono[128][128];
static int drum_samples_left[128][128];
static int drum_samples_right[128][128];
static VelocityRangeList *drum_velocity[128][128];
static char basename[80];
static char cpyrt[256];

static FILE *cfg_fd;


unsigned int freq_table[128] =
{
 8176, 8662, 9177, 9723, 
 10301, 10913, 11562, 12250, 
 12978, 13750, 14568, 15434,
 
 16352, 17324, 18354, 19445,
 20602, 21827, 23125, 24500, 
 25957, 27500, 29135, 30868, 

 32703, 34648, 36708, 38891,
 41203, 43654, 46249, 48999,
 51913, 55000, 58270, 61735,

 65406, 69296, 73416, 77782,
 82407, 87307, 92499, 97999,
 103826, 110000, 116541, 123471,

 130813, 138591, 146832, 155563,
 164814, 174614, 184997, 195998,
 207652, 220000, 233082, 246942,

 261626, 277183, 293665, 311127,
 329628, 349228, 369994, 391995,
 415305, 440000, 466164, 493883,

 523251, 554365, 587330, 622254,
 659255, 698456, 739989, 783991,
 830609, 880000, 932328, 987767,

 1046502, 1108731, 1174659, 1244508,
 1318510, 1396913, 1479978, 1567982,
 1661219, 1760000, 1864655, 1975533,

 2093005, 2217461, 2349318, 2489016,
 2637020, 2793826, 2959955, 3135963,
 3322438, 3520000, 3729310, 3951066,

 4186009, 4434922, 4698636, 4978032,
 5274041, 5587652, 5919911, 6271927,
 6644875, 7040000, 7458620, 7902133,

 8372018, 8869844, 9397273, 9956063, 
 10548082, 11175303, 11839822, 12543854
};

double bend_fine[256] = {
 1, 1.0002256593050698, 1.0004513695322617, 1.0006771306930664, 
 1.0009029427989777, 1.0011288058614922, 1.0013547198921082, 1.0015806849023274,
 1.0018067009036538, 1.002032767907594, 1.0022588859256572, 1.0024850549693551,
 1.0027112750502025, 1.0029375461797159, 1.0031638683694153, 1.0033902416308227,
 1.0036166659754628, 1.0038431414148634, 1.0040696679605541, 1.0042962456240678,
 1.0045228744169397, 1.0047495543507072, 1.0049762854369111, 1.0052030676870944,
 1.0054299011128027, 1.0056567857255843, 1.00588372153699, 1.006110708558573,
 1.0063377468018897, 1.0065648362784985, 1.0067919769999607, 1.0070191689778405,
 1.0072464122237039, 1.0074737067491204, 1.0077010525656616, 1.0079284496849015,
 1.0081558981184175, 1.008383397877789, 1.008610948974598, 1.0088385514204294,
 1.0090662052268706, 1.0092939104055114, 1.0095216669679448, 1.0097494749257656,
 1.009977334290572, 1.0102052450739643, 1.0104332072875455, 1.0106612209429215,
 1.0108892860517005, 1.0111174026254934, 1.0113455706759138, 1.0115737902145781,
 1.0118020612531047, 1.0120303838031153, 1.0122587578762337, 1.012487183484087,
 1.0127156606383041, 1.0129441893505169, 1.0131727696323602, 1.0134014014954713,
 1.0136300849514894, 1.0138588200120575, 1.0140876066888203, 1.0143164449934257,
 1.0145453349375237, 1.0147742765327674, 1.0150032697908125, 1.0152323147233171,
 1.015461411341942, 1.0156905596583505, 1.0159197596842091, 1.0161490114311862,
 1.0163783149109531, 1.0166076701351838, 1.0168370771155553, 1.0170665358637463,
 1.0172960463914391, 1.0175256087103179, 1.0177552228320703, 1.0179848887683858,
 1.0182146065309567, 1.0184443761314785, 1.0186741975816487, 1.0189040708931674,
 1.0191339960777379, 1.0193639731470658, 1.0195940021128593, 1.0198240829868295,
 1.0200542157806898, 1.0202844005061564, 1.0205146371749483, 1.0207449257987866,
 1.0209752663893958, 1.0212056589585028, 1.0214361035178368, 1.0216666000791297,
 1.0218971486541166, 1.0221277492545349, 1.0223584018921241, 1.0225891065786274,
 1.0228198633257899, 1.0230506721453596, 1.023281533049087, 1.0235124460487257,
 1.0237434111560313, 1.0239744283827625, 1.0242054977406807, 1.0244366192415495,
 1.0246677928971357, 1.0248990187192082, 1.025130296719539, 1.0253616269099028,
 1.0255930093020766, 1.0258244439078401, 1.0260559307389761, 1.0262874698072693,
 1.0265190611245079, 1.0267507047024822, 1.0269824005529853, 1.027214148687813,
 1.0274459491187637, 1.0276778018576387, 1.0279097069162415, 1.0281416643063788,
 1.0283736740398595, 1.0286057361284953, 1.0288378505841009, 1.0290700174184932,
 1.0293022366434921, 1.0295345082709197, 1.0297668323126017, 1.0299992087803651,
 1.030231637686041, 1.0304641190414621, 1.0306966528584645, 1.0309292391488862,
 1.0311618779245688, 1.0313945691973556, 1.0316273129790936, 1.0318601092816313,
 1.0320929581168212, 1.0323258594965172, 1.0325588134325767, 1.0327918199368598,
 1.0330248790212284, 1.0332579906975481, 1.0334911549776868, 1.033724371873515,
 1.0339576413969056, 1.0341909635597348, 1.0344243383738811, 1.0346577658512259,
 1.034891246003653, 1.0351247788430489, 1.0353583643813031, 1.0355920026303078,
 1.0358256936019572, 1.0360594373081489, 1.0362932337607829, 1.0365270829717617,
 1.0367609849529913, 1.0369949397163791, 1.0372289472738365, 1.0374630076372766,
 1.0376971208186156, 1.0379312868297725, 1.0381655056826686, 1.0383997773892284,
 1.0386341019613787, 1.0388684794110492, 1.0391029097501721, 1.0393373929906822,
 1.0395719291445176, 1.0398065182236185, 1.0400411602399278, 1.0402758552053915,
 1.0405106031319582, 1.0407454040315787, 1.0409802579162071, 1.0412151647977996,
 1.0414501246883161, 1.0416851375997183, 1.0419202035439705, 1.0421553225330404,
 1.042390494578898, 1.042625719693516, 1.0428609978888699, 1.043096329176938,
 1.0433317135697009, 1.0435671510791424, 1.0438026417172486, 1.0440381854960086,
 1.0442737824274138, 1.044509432523459, 1.044745135796141, 1.0449808922574599,
 1.0452167019194181, 1.0454525647940205, 1.0456884808932754, 1.0459244502291931,
 1.0461604728137874, 1.0463965486590741, 1.046632677777072, 1.0468688601798024,
 1.0471050958792898, 1.047341384887561, 1.0475777272166455, 1.047814122878576,
 1.048050571885387, 1.0482870742491166, 1.0485236299818055, 1.0487602390954964,
 1.0489969016022356, 1.0492336175140715, 1.0494703868430555, 1.0497072096012419,
 1.0499440858006872, 1.0501810154534512, 1.050417998571596, 1.0506550351671864,
 1.0508921252522903, 1.0511292688389782, 1.0513664659393229, 1.0516037165654004,
 1.0518410207292894, 1.0520783784430709, 1.0523157897188296, 1.0525532545686513,
 1.0527907730046264, 1.0530283450388465, 1.0532659706834067, 1.0535036499504049,
 1.0537413828519411, 1.0539791694001188, 1.0542170096070436, 1.0544549034848243,
 1.0546928510455722, 1.0549308523014012, 1.0551689072644284, 1.0554070159467728,
 1.0556451783605572, 1.0558833945179062, 1.0561216644309479, 1.0563599881118126,
 1.0565983655726334, 1.0568367968255465, 1.0570752818826903, 1.0573138207562065,
 1.057552413458239, 1.0577910600009348, 1.0580297603964437, 1.058268514656918,
 1.0585073227945128, 1.0587461848213857, 1.058985100749698, 1.0592240705916123
};

double bend_coarse[128] = {
 1, 1.0594630943592953, 1.122462048309373, 1.189207115002721,
 1.2599210498948732, 1.3348398541700344, 1.4142135623730951, 1.4983070768766815,
 1.5874010519681994, 1.681792830507429, 1.7817974362806785, 1.8877486253633868,
 2, 2.1189261887185906, 2.244924096618746, 2.3784142300054421,
 2.5198420997897464, 2.6696797083400687, 2.8284271247461903, 2.996614153753363,
 3.1748021039363992, 3.363585661014858, 3.5635948725613571, 3.7754972507267741,
 4, 4.2378523774371812, 4.4898481932374912, 4.7568284600108841,
 5.0396841995794928, 5.3393594166801366, 5.6568542494923806, 5.993228307506727,
 6.3496042078727974, 6.727171322029716, 7.1271897451227151, 7.5509945014535473,
 8, 8.4757047548743625, 8.9796963864749824, 9.5136569200217682,
 10.079368399158986, 10.678718833360273, 11.313708498984761, 11.986456615013454,
 12.699208415745595, 13.454342644059432, 14.25437949024543, 15.101989002907095,
 16, 16.951409509748721, 17.959392772949972, 19.027313840043536,
 20.158736798317967, 21.357437666720553, 22.627416997969522, 23.972913230026901,
 25.398416831491197, 26.908685288118864, 28.508758980490853, 30.203978005814196,
 32, 33.902819019497443, 35.918785545899944, 38.054627680087073,
 40.317473596635935, 42.714875333441107, 45.254833995939045, 47.945826460053802,
 50.796833662982394, 53.817370576237728, 57.017517960981706, 60.407956011628393,
 64, 67.805638038994886, 71.837571091799887, 76.109255360174146,
 80.63494719327187, 85.429750666882214, 90.509667991878089, 95.891652920107603,
 101.59366732596479, 107.63474115247546, 114.03503592196341, 120.81591202325679,
 128, 135.61127607798977, 143.67514218359977, 152.21851072034829,
 161.26989438654374, 170.85950133376443, 181.01933598375618, 191.78330584021521,
 203.18733465192958, 215.26948230495091, 228.07007184392683, 241.63182404651357,
 256, 271.22255215597971, 287.35028436719938, 304.43702144069658,
 322.53978877308765, 341.71900266752868, 362.03867196751236, 383.56661168043064,
 406.37466930385892, 430.53896460990183, 456.14014368785394, 483.26364809302686,
 512, 542.44510431195943, 574.70056873439876, 608.87404288139317,
 645.0795775461753, 683.43800533505737, 724.07734393502471, 767.13322336086128,
 812.74933860771785, 861.07792921980365, 912.28028737570787, 966.52729618605372,
 1024, 1084.8902086239189, 1149.4011374687975, 1217.7480857627863,
 1290.1591550923506, 1366.8760106701147, 1448.1546878700494, 1534.2664467217226
};

typedef union {
      unsigned char c[4];
      unsigned long l;
} long_end;
typedef union {
      unsigned char c[2];
      unsigned short s;
} short_end;

static int big_endian = 1;

static long longnum(unsigned char c1, unsigned char c2,
		unsigned char c3, unsigned char c4)
{
    long_end lswitch;
    if (big_endian) {
	lswitch.c[0] = c4;
	lswitch.c[1] = c3;
	lswitch.c[2] = c2;
	lswitch.c[3] = c1;
    } else {
	lswitch.c[0] = c1;
	lswitch.c[1] = c2;
	lswitch.c[2] = c3;
	lswitch.c[3] = c4;
    }
    return(lswitch.l);
}

static short shortnum(unsigned char c1, unsigned char c2)
{
    short_end sswitch;
    if (big_endian) {
	sswitch.c[0] = c2;
	sswitch.c[1] = c1;
    } else {
	sswitch.c[0] = c1;
	sswitch.c[1] = c2;
    }
    return(sswitch.s);
}

static void byteorder(void)
{   long_end hexx;

    hexx.c[0] = 0x12;
    hexx.c[1] = 0x34;
    hexx.c[2] = 0x56;
    hexx.c[3] = 0x78;
    if (hexx.l == 0x78563412) big_endian = 0;
}


static char *getname(char *p)
{
	int i, j, e;
	static char buf[21];
	strncpy(buf, p, 20);
	buf[20] = 0;
	for (i = 19; i > 4 && buf[i]==' '; i--) {
	  buf[i] = 0;
	}
	e = i + 1;
	if (e < 5) return buf;
	for (i = 0; i < e; i++) {
	  if (buf[i] == '/') {
		  if (i) buf[i] = '.';
		  else buf[i] = ' ';
	  }
	  else if (buf[i] == '\\') buf[i] = ' ';
	  else if (buf[i] == '#') buf[i] = ' ';
	  else if (buf[i] == '|') buf[i] = ' ';
	  else if (buf[i] == '&') buf[i] = ' ';
	  else if (buf[i] == '*') buf[i] = ' ';
	  else if (buf[i] == '!') buf[i] = ' ';
	  else if (buf[i] == '\'') buf[i] = ' ';
	  else if (buf[i] == '"') buf[i] = ' ';
	  else if (buf[i] == '?') buf[i] = ' ';
	  else if (buf[i] == '~') buf[i] = ' ';
	  else if (buf[i] == '[') buf[i] = '-';
	  else if (buf[i] == ']') buf[i] = ' ';
	  else if (buf[i] == '(') buf[i] = '-';
	  else if (buf[i] == ')') buf[i] = ' ';
	}
	for (i = 0; i < e; i++) {
	  if (buf[i] == ' ') {
	    for (j = i; j < e; j++)
	      buf[j] = buf[j+1];
	    e--;
	  }
	}
	for (i = 0; i < e; i++) {
	  if (buf[i] == ' ') {
	    for (j = i; j < e; j++)
	      buf[j] = buf[j+1];
	    e--;
	  }
	}
	e = strlen(buf);
	while (e > 3 && buf[e-1] == ' ') {
		buf[e-1] = '\0';
		e--;
	}
	return buf;
}



/* scratch buffer for generating new patch files */
static unsigned char *mem = NULL;
static int mem_size = 0;
static int mem_alloced = 0;



/* writes a byte to the memory buffer */
static void mem_write8(int val)
{
   if (mem_size >= mem_alloced) {
      mem_alloced += 4096;
      if (!(mem = realloc(mem, mem_alloced))) {
	      fprintf(stderr, "Memory allocation of %d failed with mem size %d\n", mem_alloced, mem_size);
	      exit(1);
      }
   }

   mem[mem_size] = val;
   mem_size++;
}



/* writes a word to the memory buffer (little endian) */
static void mem_write16(int val)
{
 if (big_endian) {
   mem_write8((val >> 8) & 0xFF);
   mem_write8(val & 0xFF);
 }
 else {
   mem_write8(val & 0xFF);
   mem_write8((val >> 8) & 0xFF);
 }
}



/* writes a long to the memory buffer (little endian) */
static void mem_write32(int val)
{
 if (big_endian) {
   mem_write8((val >> 24) & 0xFF);
   mem_write8((val >> 16) & 0xFF);
   mem_write8((val >> 8) & 0xFF);
   mem_write8(val & 0xFF);
 }
 else {
   mem_write8(val & 0xFF);
   mem_write8((val >> 8) & 0xFF);
   mem_write8((val >> 16) & 0xFF);
   mem_write8((val >> 24) & 0xFF);
 }
}



/* alters data already written to the memory buffer (little endian) */
static void mem_modify32(int pos, int val)
{
 if (big_endian) {
   mem[pos+3] = (val >> 24) & 0xFF;
   mem[pos+2] = (val >> 16) & 0xFF;
   mem[pos+1] = (val >> 8) & 0xFF;
   mem[pos] = val & 0xFF;
 }
 else {
   mem[pos] = val & 0xFF;
   mem[pos+1] = (val >> 8) & 0xFF;
   mem[pos+2] = (val >> 16) & 0xFF;
   mem[pos+3] = (val >> 24) & 0xFF;
 }
}



/* writes a block of data the memory buffer */
static void mem_write_block(void *data, int size)
{
   if (mem_size+size > mem_alloced) {
      mem_alloced = (mem_alloced + size + 4095) & ~4095;
      if (!(mem = realloc(mem, mem_alloced))) {
	      fprintf(stderr, "Memory allocation of %d failed with mem size %d\n", mem_alloced, mem_size);
	      exit(1);
      }
   }

   memcpy(mem+mem_size, data, size);
   mem_size += size;
}




/* SoundFont chunk format and ID values */
typedef struct RIFF_CHUNK
{
   int size;
   int id;
   int type;
   int end;
} RIFF_CHUNK;


#define CID(a,b,c,d)    (((d)<<24)+((c)<<16)+((b)<<8)+((a)))


#define CID_RIFF  CID('R','I','F','F')
#define CID_LIST  CID('L','I','S','T')
#define CID_INFO  CID('I','N','F','O')
#define CID_sdta  CID('s','d','t','a')
#define CID_snam  CID('s','n','a','m')
#define CID_smpl  CID('s','m','p','l')
#define CID_pdta  CID('p','d','t','a')
#define CID_phdr  CID('p','h','d','r')
#define CID_pbag  CID('p','b','a','g')
#define CID_pmod  CID('p','m','o','d')
#define CID_pgen  CID('p','g','e','n')
#define CID_inst  CID('i','n','s','t')
#define CID_ibag  CID('i','b','a','g')
#define CID_imod  CID('i','m','o','d')
#define CID_igen  CID('i','g','e','n')
#define CID_shdr  CID('s','h','d','r')
#define CID_ifil  CID('i','f','i','l')
#define CID_isng  CID('i','s','n','g')
#define CID_irom  CID('i','r','o','m')
#define CID_iver  CID('i','v','e','r')
#define CID_INAM  CID('I','N','A','M')
#define CID_IPRD  CID('I','P','R','D')
#define CID_ICOP  CID('I','C','O','P')
#define CID_sfbk  CID('s','f','b','k')
#define CID_ICRD  CID('I','C','R','D')
#define CID_IENG  CID('I','E','N','G')
#define CID_ICMT  CID('I','C','M','T')
#define CID_ISFT  CID('I','S','F','T')



/* SoundFont generator types */
#define SFGEN_startAddrsOffset         0
#define SFGEN_endAddrsOffset           1
#define SFGEN_startloopAddrsOffset     2
#define SFGEN_endloopAddrsOffset       3
#define SFGEN_startAddrsCoarseOffset   4
#define SFGEN_modLfoToPitch            5
#define SFGEN_vibLfoToPitch            6
#define SFGEN_modEnvToPitch            7
#define SFGEN_initialFilterFc          8
#define SFGEN_initialFilterQ           9
#define SFGEN_modLfoToFilterFc         10
#define SFGEN_modEnvToFilterFc         11
#define SFGEN_endAddrsCoarseOffset     12
#define SFGEN_modLfoToVolume           13
#define SFGEN_unused1                  14
#define SFGEN_chorusEffectsSend        15
#define SFGEN_reverbEffectsSend        16
#define SFGEN_pan                      17
#define SFGEN_unused2                  18
#define SFGEN_unused3                  19
#define SFGEN_unused4                  20
#define SFGEN_delayModLFO              21
#define SFGEN_freqModLFO               22
#define SFGEN_delayVibLFO              23
#define SFGEN_freqVibLFO               24
#define SFGEN_delayModEnv              25
#define SFGEN_attackModEnv             26
#define SFGEN_holdModEnv               27
#define SFGEN_decayModEnv              28
#define SFGEN_sustainModEnv            29
#define SFGEN_releaseModEnv            30
#define SFGEN_keynumToModEnvHold       31
#define SFGEN_keynumToModEnvDecay      32
#define SFGEN_delayVolEnv              33
#define SFGEN_attackVolEnv             34
#define SFGEN_holdVolEnv               35
#define SFGEN_decayVolEnv              36
#define SFGEN_sustainVolEnv            37
#define SFGEN_releaseVolEnv            38
#define SFGEN_keynumToVolEnvHold       39
#define SFGEN_keynumToVolEnvDecay      40
#define SFGEN_instrument               41
#define SFGEN_reserved1                42
#define SFGEN_keyRange                 43
#define SFGEN_velRange                 44
#define SFGEN_startloopAddrsCoarse     45
#define SFGEN_keynum                   46
#define SFGEN_velocity                 47
#define SFGEN_initialAttenuation       48
#define SFGEN_reserved2                49
#define SFGEN_endloopAddrsCoarse       50
#define SFGEN_coarseTune               51
#define SFGEN_fineTune                 52
#define SFGEN_sampleID                 53
#define SFGEN_sampleModes              54
#define SFGEN_reserved3                55
#define SFGEN_scaleTuning              56
#define SFGEN_exclusiveClass           57
#define SFGEN_overridingRootKey        58
#define SFGEN_unused5                  59
#define SFGEN_endOper                  60


/*================================================================
 *	SoundFont layer structure
 *================================================================*/

enum {
	SF_startAddrs,		/* 0 sample start address -4 (0to*0xffffff)*/
        SF_endAddrs,		/* 1 */
        SF_startloopAddrs,	/* 2 loop start address -4 (0 to * 0xffffff) */
        SF_endloopAddrs,	/* 3 loop end address -3 (0 to * 0xffffff) */
        SF_startAddrsHi,	/* 4 high word of startAddrs */
        SF_lfo1ToPitch,		/* 5 main fm: lfo1-> pitch */
        SF_lfo2ToPitch,		/* 6 aux fm:  lfo2-> pitch */
        SF_env1ToPitch,		/* 7 pitch env: env1(aux)-> pitch */
        SF_initialFilterFc,	/* 8 initial filter cutoff */
        SF_initialFilterQ,	/* 9 filter Q */
        SF_lfo1ToFilterFc,	/* 10 filter modulation: lfo1->filter*cutoff */
        SF_env1ToFilterFc,	/* 11 filter env: env1(aux)->filter * cutoff */
        SF_endAddrsHi,		/* 12 high word of endAddrs */
        SF_lfo1ToVolume,	/* 13 tremolo: lfo1-> volume */
        SF_env2ToVolume,	/* 14 Env2Depth: env2-> volume */
        SF_chorusEffectsSend,	/* 15 chorus */
        SF_reverbEffectsSend,	/* 16 reverb */
        SF_panEffectsSend,	/* 17 pan */
        SF_auxEffectsSend,	/* 18 pan auxdata (internal) */
        SF_sampleVolume,	/* 19 used internally */
        SF_unused3,		/* 20 */
        SF_delayLfo1,		/* 21 delay 0x8000-n*(725us) */
        SF_freqLfo1,		/* 22 frequency */
        SF_delayLfo2,		/* 23 delay 0x8000-n*(725us) */
        SF_freqLfo2,		/* 24 frequency */
        SF_delayEnv1,		/* 25 delay 0x8000 - n(725us) */
        SF_attackEnv1,		/* 26 attack */
        SF_holdEnv1,		/* 27 hold */
        SF_decayEnv1,		/* 28 decay */
        SF_sustainEnv1,		/* 29 sustain */
        SF_releaseEnv1,		/* 30 release */
        SF_autoHoldEnv1,	/* 31 */
        SF_autoDecayEnv1,	/* 32 */
        SF_delayEnv2,		/* 33 delay 0x8000 - n(725us) */
        SF_attackEnv2,		/* 34 attack */
        SF_holdEnv2,		/* 35 hold */
        SF_decayEnv2,		/* 36 decay */
        SF_sustainEnv2,		/* 37 sustain */
        SF_releaseEnv2,		/* 38 release */
        SF_autoHoldEnv2,	/* 39 */
        SF_autoDecayEnv2,	/* 40 */
        SF_instrument,		/* 41 */
        SF_nop,			/* 42 */
        SF_keyRange,		/* 43 */
        SF_velRange,		/* 44 */
        SF_startloopAddrsHi,	/* 45 high word of startloopAddrs */
        SF_keynum,		/* 46 */
        SF_velocity,		/* 47 */
        SF_initAtten,		/* 48 */
        SF_keyTuning,		/* 49 */
        SF_endloopAddrsHi,	/* 50 high word of endloopAddrs */
        SF_coarseTune,		/* 51 */
        SF_fineTune,		/* 52 */
        SF_sampleId,		/* 53 */
        SF_sampleFlags,		/* 54 */
        SF_samplePitch,		/* 55 SF1 only */
        SF_scaleTuning,		/* 56 */
        SF_keyExclusiveClass,	/* 57 */
        SF_rootKey,		/* 58 */
	SF_EOF			/* 59 */
};

/* SoundFont sample data */
static short *sf_sample_data = NULL;
static int sf_sample_data_size = 0;



/* SoundFont preset headers */
typedef struct sfPresetHeader
{
   char achPresetName[20];
   unsigned short wPreset;
   unsigned short wBank;
   unsigned short wPresetBagNdx;
   unsigned long dwLibrary;
   unsigned long dwGenre;
   unsigned long dwMorphology;
} sfPresetHeader;


static sfPresetHeader *sf_presets = NULL;
static int sf_num_presets = 0;



/* SoundFont preset indexes */
typedef struct sfPresetBag
{
   unsigned short wGenNdx;
   unsigned short wModNdx;
} sfPresetBag;


static sfPresetBag *sf_preset_indexes = NULL;
static int sf_num_preset_indexes = 0;



/* SoundFont preset generators */
typedef struct rangesType
{
   unsigned char byLo;
   unsigned char byHi;
} rangesType;


typedef union genAmountType
{
   rangesType ranges;
   short shAmount;
   unsigned short wAmount;
} genAmountType;


typedef struct sfGenList
{
   unsigned short sfGenOper;
   genAmountType genAmount;
} sfGenList;


static sfGenList *sf_preset_generators = NULL;
static int sf_num_preset_generators = 0;



/* SoundFont instrument headers */
typedef struct sfInst
{
   char achInstName[20];
   unsigned short wInstBagNdx;
} sfInst;


static sfInst *sf_instruments = NULL;
static int sf_num_instruments = 0;



/* SoundFont instrument indexes */
typedef struct sfInstBag
{
   unsigned short wInstGenNdx;
   unsigned short wInstModNdx;
} sfInstBag;


static sfInstBag *sf_instrument_indexes = NULL;
static int sf_num_instrument_indexes = 0;



/* SoundFont instrument generators */
static sfGenList *sf_instrument_generators = NULL;
static int sf_num_instrument_generators = 0;



/* SoundFont sample headers */
typedef struct sfSample
{
   char achSampleName[20];
   unsigned long dwStart;
   unsigned long dwEnd;
   unsigned long dwStartloop;
   unsigned long dwEndloop;
   unsigned long dwSampleRate;
   unsigned char byOriginalKey;
   signed char chCorrection;
   unsigned short wSampleLink;
   unsigned short sfSampleType;		/* 1 mono,2 right,4 left,linked 8,0x8000=ROM */
} sfSample;


static sfSample *sf_samples = NULL;
static int sf_num_samples = 0;



/* list of the layers waiting to be dealt with */
typedef struct EMPTY_WHITE_ROOM
{
   sfSample *sample;
   sfGenList *igen;
   sfGenList *pgen;
   sfGenList *global_izone;
   sfGenList *global_pzone;
   int igen_count;
   int pgen_count;
   int global_izone_count;
   int global_pzone_count;
   float volume;
   int stereo_mode;
} EMPTY_WHITE_ROOM;

#define MAX_WAITING  256

static EMPTY_WHITE_ROOM waiting_list[MAX_WAITING];

static int waiting_list_count;

/* SoundFont parameters for the current sample */
static int sf_instrument_look_index;
static int sf_instrument_unused5;
static int sf_sample_look_index;
static int sf_start, sf_end;
static int sf_loop_start, sf_loop_end;
static int sf_key, sf_tune;
static int sf_pan;
static int sf_keyscale;
static int sf_keymin, sf_keymax;
static int sf_velmin, sf_velmax;
static int sf_sustain_mod_env;
static int sf_mod_env_to_pitch;
static int sf_delay_vol_env;
static int sf_attack_vol_env;
static int sf_hold_vol_env;
static int sf_decay_vol_env;
static int sf_release_vol_env;
static int sf_sustain_level;
static int sf_mode;

static int sp_freq_center;
static int sf_exclusiveClass;

static int sf_chorusEffectsSend;
static int sf_reverbEffectsSend;
static int sf_initialAttenuation;
static int sf_modLfoToPitch;
static int sf_vibLfoToPitch;
static int sf_velocity;
static int sf_keynum;
static int sf_keynumToModEnvHold;
static int sf_keynumToModEnvDecay;
static int sf_keynumToVolEnvHold;
static int sf_keynumToVolEnvDecay;
static int sf_modLfoToVolume;
static int sf_delayModLFO;
static int sp_delayModLFO;
static int sf_freqModLFO;
static int sf_delayVibLFO;
static int sf_freqVibLFO;
static int sf_delayModEnv;
static int sf_attackModEnv;
static int sf_holdModEnv;
static int sf_decayModEnv;
static int sf_releaseModEnv;

static short sf_initialFilterQ;
static short sf_initialFilterFc;
static short sf_modEnvToFilterFc;
static short sf_modLfoToFilterFc;
static short sp_resonance;
static short sp_modEnvToFilterFc;
static short sp_modLfoToFilterFc;
static short sp_modEnvToPitch;
static short sp_cutoff_freq;
static int sp_vibrato_depth, sp_vibrato_delay;
static unsigned char sp_vibrato_control_ratio, sp_vibrato_sweep_increment;
static int sp_tremolo_depth;
static unsigned char sp_tremolo_phase_increment, sp_tremolo_sweep_increment;
static int sp_lfo_depth;
static short sp_lfo_phase_increment;
	/* sp_lfo_phase_increment is actually frequency */

/* interprets a SoundFont generator object */
static void apply_generator(sfGenList *g, int preset, int global)
{
   switch (g->sfGenOper) {

      case SFGEN_startAddrsOffset:
	 sf_start += g->genAmount.shAmount;
	 break;

      case SFGEN_endAddrsOffset:
	 sf_end += g->genAmount.shAmount;
	 break;

      case SFGEN_startloopAddrsOffset:
	 sf_loop_start += g->genAmount.shAmount;
	 break;

      case SFGEN_endloopAddrsOffset:
	 sf_loop_end += g->genAmount.shAmount;
	 break;

      case SFGEN_startAddrsCoarseOffset:
	 sf_start += (int)g->genAmount.shAmount * 32768;
	 break;

      case SFGEN_endAddrsCoarseOffset:
	 sf_end += (int)g->genAmount.shAmount * 32768;
	 break;

      case SFGEN_startloopAddrsCoarse:
	 sf_loop_start += (int)g->genAmount.shAmount * 32768;
	 break;

      case SFGEN_endloopAddrsCoarse:
	 sf_loop_end += (int)g->genAmount.shAmount * 32768;
	 break;

      case SFGEN_modEnvToPitch:
	 if (preset)
	    sf_mod_env_to_pitch += g->genAmount.shAmount;
	 else
	    sf_mod_env_to_pitch = g->genAmount.shAmount;
	 break;

      case SFGEN_sustainModEnv:
	 if (preset)
	    sf_sustain_mod_env += g->genAmount.shAmount;
	 else
	    sf_sustain_mod_env = g->genAmount.shAmount;
	 break;

      case SFGEN_delayVolEnv:
	 if (preset)
	    sf_delay_vol_env += g->genAmount.shAmount;
	 else
	    sf_delay_vol_env = g->genAmount.shAmount;
	 break;

      case SFGEN_attackVolEnv:
	 if (preset)
	    sf_attack_vol_env += g->genAmount.shAmount;
	 else
	    sf_attack_vol_env = g->genAmount.shAmount;
	 break;

      case SFGEN_holdVolEnv:
	 if (preset)
	    sf_hold_vol_env += g->genAmount.shAmount;
	 else
	    sf_hold_vol_env = g->genAmount.shAmount;
	 break;

      case SFGEN_decayVolEnv:
	 if (preset)
	    sf_decay_vol_env += g->genAmount.shAmount;
	 else
	    sf_decay_vol_env = g->genAmount.shAmount;
	 break;

      case SFGEN_sustainVolEnv:
	 if (preset)
	    sf_sustain_level += g->genAmount.shAmount;
	 else
	    sf_sustain_level = g->genAmount.shAmount;
	 break;

      case SFGEN_releaseVolEnv:
	 if (preset)
	    sf_release_vol_env += g->genAmount.shAmount;
	 else
	    sf_release_vol_env = g->genAmount.shAmount;
	 break;

      case SFGEN_pan:
	 if (preset)
	    sf_pan += g->genAmount.shAmount;
	 else
	    sf_pan = g->genAmount.shAmount;
	 break;

      case SFGEN_keyRange:
	 if (preset) {
	     if (g->genAmount.ranges.byLo >= sf_keymin && g->genAmount.ranges.byHi <= sf_keymax) {
	         sf_keymin = g->genAmount.ranges.byLo;
	         sf_keymax = g->genAmount.ranges.byHi;
	     }
	 }
	 else {
	     sf_keymin = g->genAmount.ranges.byLo;
	     sf_keymax = g->genAmount.ranges.byHi;
	 }
	 break;

      case SFGEN_velRange:
	 if (preset) {
	     if (g->genAmount.ranges.byLo >= sf_velmin && g->genAmount.ranges.byHi <= sf_velmax) {
	         sf_velmin = g->genAmount.ranges.byLo;
	         sf_velmax = g->genAmount.ranges.byHi;
	     }
	 }
	 else {
	     sf_velmin = g->genAmount.ranges.byLo;
	     sf_velmax = g->genAmount.ranges.byHi;
	 }
	 break;

      case SFGEN_coarseTune:
	 if (preset)
	    sf_tune += g->genAmount.shAmount * 100;
	 else
	    sf_tune = g->genAmount.shAmount * 100;
	 break;

      case SFGEN_fineTune:
	 if (preset)
	    sf_tune += g->genAmount.shAmount;
	 else
	    sf_tune = g->genAmount.shAmount;
	 break;

      case SFGEN_sampleModes:
	 sf_mode = g->genAmount.wAmount;
	 break;

      case SFGEN_scaleTuning:
	 if (preset)
	    sf_keyscale += g->genAmount.shAmount;
	 else
	    sf_keyscale = g->genAmount.shAmount;
	 break;

      case SFGEN_overridingRootKey:
	 if (g->genAmount.shAmount >= 0 && g->genAmount.shAmount <= 127)
	    sf_key = g->genAmount.shAmount;
	 break;

      case SFGEN_exclusiveClass:
	    sf_exclusiveClass = g->genAmount.shAmount;
	 break;

      case SFGEN_initialAttenuation:
	 if (preset)
	    sf_initialAttenuation += g->genAmount.shAmount;
	 else
	    sf_initialAttenuation = g->genAmount.shAmount;
	 break;
      case SFGEN_chorusEffectsSend:
	 if (preset)
	    sf_chorusEffectsSend += g->genAmount.shAmount;
	 else
	    sf_chorusEffectsSend = g->genAmount.shAmount;
	 break;
      case SFGEN_reverbEffectsSend:
	 if (preset)
	    sf_reverbEffectsSend += g->genAmount.shAmount;
	 else
	    sf_reverbEffectsSend = g->genAmount.shAmount;
	 break;
      case SFGEN_modLfoToPitch:
	 if (preset)
	    sf_modLfoToPitch += g->genAmount.shAmount;
	 else
	    sf_modLfoToPitch = g->genAmount.shAmount;
	 break;
      case SFGEN_vibLfoToPitch:
	 if (preset)
	    sf_vibLfoToPitch += g->genAmount.shAmount;
	 else
	    sf_vibLfoToPitch = g->genAmount.shAmount;
	 break;
      case SFGEN_velocity:
	    sf_velocity = g->genAmount.shAmount;
	 break;
      case SFGEN_keynum:
	    sf_keynum = g->genAmount.shAmount;
	 break;
      case SFGEN_keynumToModEnvHold:
	 if (preset)
	    sf_keynumToModEnvHold += g->genAmount.shAmount;
	 else
	    sf_keynumToModEnvHold = g->genAmount.shAmount;
	 break;
      case SFGEN_keynumToModEnvDecay:
	 if (preset)
	    sf_keynumToModEnvDecay += g->genAmount.shAmount;
	 else
	    sf_keynumToModEnvDecay = g->genAmount.shAmount;
	 break;
      case SFGEN_keynumToVolEnvHold:
	 if (preset)
	    sf_keynumToVolEnvHold += g->genAmount.shAmount;
	 else
	    sf_keynumToVolEnvHold = g->genAmount.shAmount;
	 break;
      case SFGEN_keynumToVolEnvDecay:
	 if (preset)
	    sf_keynumToVolEnvDecay += g->genAmount.shAmount;
	 else
	    sf_keynumToVolEnvDecay = g->genAmount.shAmount;
	 break;
      case SFGEN_modLfoToVolume:
	 if (preset)
	    sf_modLfoToVolume += g->genAmount.shAmount;
	 else
	    sf_modLfoToVolume = g->genAmount.shAmount;
	 break;
      case SFGEN_delayModLFO:
	 if (preset)
	    sf_delayModLFO += g->genAmount.shAmount;
	 else
	    sf_delayModLFO = g->genAmount.shAmount;
	 break;
      case SFGEN_freqModLFO:
	 if (preset)
	    sf_freqModLFO += g->genAmount.shAmount;
	 else
	    sf_freqModLFO = g->genAmount.shAmount;
	 break;
      case SFGEN_delayVibLFO:
	 if (preset)
	    sf_delayVibLFO += g->genAmount.shAmount;
	 else
	    sf_delayVibLFO = g->genAmount.shAmount;
	 break;
      case SFGEN_freqVibLFO:
	 if (preset)
	    sf_freqVibLFO += g->genAmount.shAmount;
	 else
	    sf_freqVibLFO = g->genAmount.shAmount;
	 break;
      case SFGEN_delayModEnv:
	 if (preset)
	    sf_delayModEnv += g->genAmount.shAmount;
	 else
	    sf_delayModEnv = g->genAmount.shAmount;
	 break;
      case SFGEN_attackModEnv:
	 if (preset)
	    sf_attackModEnv += g->genAmount.shAmount;
	 else
	    sf_attackModEnv = g->genAmount.shAmount;
	 break;
      case SFGEN_holdModEnv:
	 if (preset)
	    sf_holdModEnv += g->genAmount.shAmount;
	 else
	    sf_holdModEnv = g->genAmount.shAmount;
	 break;
      case SFGEN_decayModEnv:
	 if (preset)
	    sf_decayModEnv += g->genAmount.shAmount;
	 else
	    sf_decayModEnv = g->genAmount.shAmount;
	 break;
      case SFGEN_releaseModEnv:
	 if (preset)
	    sf_releaseModEnv += g->genAmount.shAmount;
	 else
	    sf_releaseModEnv = g->genAmount.shAmount;
	 break;
      case SFGEN_initialFilterQ:
	 if (preset)
	    sf_initialFilterQ += g->genAmount.shAmount;
	 else
	    sf_initialFilterQ = g->genAmount.shAmount;
	 break;
      case SFGEN_initialFilterFc:
	 if (preset)
	    sf_initialFilterFc += g->genAmount.shAmount;
	 else
	    sf_initialFilterFc = g->genAmount.shAmount;
	 break;
      case SFGEN_modEnvToFilterFc:
	 if (preset)
	    sf_modEnvToFilterFc += g->genAmount.shAmount;
	 else
	    sf_modEnvToFilterFc = g->genAmount.shAmount;
	 break;
      case SFGEN_modLfoToFilterFc:
	 if (preset)
	    sf_modLfoToFilterFc += g->genAmount.shAmount;
	 else
	    sf_modLfoToFilterFc = g->genAmount.shAmount;
	 break;
      case SFGEN_instrument:
	 sf_instrument_look_index = g->genAmount.shAmount;
	 break;
      case SFGEN_sampleID:
	 sf_sample_look_index = g->genAmount.shAmount;
	 break;
      case SFGEN_unused5:
	 sf_instrument_unused5 = g->genAmount.shAmount;
	 if (opt_verbose) printf("APS parameter %d\n", sf_instrument_unused5);
	 break;
      default:
	 fprintf(stderr,"Warning: generator %d with value %d not handled at the %s %s level\n",
			 g->sfGenOper, g->genAmount.shAmount,
			 global? "global":"local",
			 preset? "preset":"instrument");
	 break;
   }
}



/* converts AWE32 (MIDI) pitches to GUS (frequency) format */
static int key2freq(int note, int cents)
{
   return pow(2.0, (float)(note*100+cents)/1200.0) * 8175.800781;
}



/* converts the strange AWE32 timecent values to milliseconds */
static int timecent2msec(int t)
{
   double msec;
   msec = (double)(1000 * pow(2.0, (double)( t ) / 1200.0));
   return (int)msec;
}



/* converts milliseconds to the even stranger floating point GUS format */
static int msec2gus(int t, int r)
{
   static int vexp[4] = { 1, 8, 64, 512 };
   int e, m;

   if (r <= 0)
      return 0x3F;

   t = t * 32 / r;

   if (t <= 0)
      return 0x3F;

   for (e=3; e>=0; e--) {
      m = (vexp[e] * 16 + t/2) / t;

      if ((m > 0) && (m < 64))
	 return ((e << 6) | m);
   }

   return 0xC1;
}

/* Bits in modes: */
#define MODES_16BIT	(1<<0)
#define MODES_UNSIGNED	(1<<1)
#define MODES_LOOPING	(1<<2)
#define MODES_PINGPONG	(1<<3)
#define MODES_REVERSE	(1<<4)
#define MODES_SUSTAIN	(1<<5)
#define MODES_ENVELOPE	(1<<6)
#define MODES_FAST_RELEASE	(1<<7)

/* The sampleFlags value, in my experience, is not to be trusted,
 * so the following uses some heuristics to guess whether to
 * set looping and sustain modes. (gl)
 */
static int getmodes(int sampleFlags, int program, int banknum)
{
	int modes;
	int orig_sampleFlags = sampleFlags;

	modes = MODES_ENVELOPE;

      if (opt_8bit)
	 modes |= MODES_UNSIGNED;                      /* signed waveform */
      else
	 modes |= MODES_16BIT;                      /* 16-bit waveform */


	if (sampleFlags == 3) modes |= MODES_FAST_RELEASE;

	/* arbitrary adjustments (look at sustain of vol envelope? ) */

      if (opt_adjust_sample_flags) {

	if (sampleFlags && sf_sustain_mod_env == 0) sampleFlags = 3;
	else if (sampleFlags && sf_sustain_mod_env >= 1000) sampleFlags = 1;
	else if (banknum != 128 && sampleFlags == 1) {
		/* organs, accordians */
		if (program >= 16 && program <= 23) sampleFlags = 3;
		/* strings */
		else if (program >= 40 && program <= 44) sampleFlags = 3;
		/* strings, voice */
		else if (program >= 48 && program <= 54) sampleFlags = 3;
		/* horns, woodwinds */
		else if (program >= 56 && program <= 79) sampleFlags = 3;
		/* lead, pad, fx */
		else if (program >= 80 && program <=103) sampleFlags = 3;
		/* bagpipe, fiddle, shanai */
		else if (program >=109 && program <=111) sampleFlags = 3;
		/* breath noise, ... telephone, helicopter */
		else if (program >=121 && program <=125) sampleFlags = 3;
		/* applause */
		else if (program ==126) sampleFlags = 3;
	}

	if (opt_verbose && orig_sampleFlags != sampleFlags)
		printf("changed sampleFlags from %d to %d\n",
				orig_sampleFlags, sampleFlags);
      }
      else if (sampleFlags == 1) sampleFlags = 3;

	if (sampleFlags == 1 || sampleFlags == 3)
		modes |= MODES_LOOPING;
	if (sampleFlags == 3)
		modes |= MODES_SUSTAIN;
	return modes;
}


/* calculate root pitch */
/* This code is derived from some version of Timidity++ and comes
 * from Takashi Iwai and/or Masanao Izumi (who are not responsible
 * for my interpretation of it). (gl)
 */
static int calc_root_pitch(void)
{
	int root, tune;

	root = sf_key; /* sample originalPitch */
	tune = sf_tune; /* sample pitchCorrection */

	/* tuning */
	/*tune += sf_keyscale; Why did I say this? */
/* ??
		tune += lay->val[SF_coarseTune] * 100
			+ lay->val[SF_fineTune];
*/

	/* it's too high.. */
	if (root >= sf_keymax + 60)
		root -= 60;
/*
	if (lay->set[SF_keyRange] &&
	    root >= HI_VAL(lay->val[SF_keyRange]) + 60)
		root -= 60;

*/

	while (tune <= -100) {
		root++;
		tune += 100;
	}
	while (tune > 0) {
		root--;
		tune -= 100;
	}

    if (root > 0) sp_freq_center = root;
    else sp_freq_center = 60;

    tune = (-tune * 256) / 100;

    if (root > 127)
	return (int)((double)freq_table[127] *
				  bend_coarse[root - 127] * bend_fine[tune]);
    else if (root < 0)
	return (int)((double)freq_table[0] /
				  bend_coarse[-root] * bend_fine[tune]);
    else
	return (int)((double)freq_table[root] * bend_fine[tune]);

}


#define CB_TO_VOLUME(centibel) (255 * (1.0 - ((double)(centibel)/100.0) / (1200.0 * log10(2.0)) ))
/* convert peak volume to linear volume (0-255) */
static unsigned int calc_volume(void)
{
	int v;
	double ret;

	if (!sf_initialAttenuation) return 255;
	v = sf_initialAttenuation;
	if (v < 0) v = 0;
	else if (v > 960) v = 960;
	ret = CB_TO_VOLUME((double)v);
	if (ret < 1.0) return 0;
	if (ret > 255.0) return 255;
	return (unsigned int)ret;
}


#define TO_VOLUME(centibel) (unsigned char)(255 * pow(10.0, -(double)(centibel)/200.0))
/* convert sustain volume to linear volume */
static unsigned char calc_sustain(void)
{
	int level;

	if (!sf_sustain_level) return 250;
	level = TO_VOLUME(sf_sustain_level);
	if (level > 253) level = 253;
	if (level < 100) level = 250; /* Protect against bogus value? This is for PC42c saxes. */
	return (unsigned char)level;
}
static unsigned int calc_mod_sustain(void)
{
	if (!sf_sustain_mod_env) return 250;
	return TO_VOLUME(sf_sustain_mod_env);
}
static void calc_resonance(void)
{
	short val = sf_initialFilterQ;
	//sp_resonance = pow(10.0, (double)val / 2.0 / 200.0) - 1;
	sp_resonance = val;
	if (sp_resonance < 0)
		sp_resonance = 0;
}

#define TO_HZ(abscents) (int)(8.176 * pow(2.0,(double)(abscents)/1200.0))
#define TO_HZ20(abscents) (int)(20 * 8.176 * pow(2.0,(double)(abscents)/1200.0))
/* calculate cutoff/resonance frequency */
static void calc_cutoff(void)
{
	short val;

	if (sf_initialFilterFc < 1) val = 13500;
	else val = sf_initialFilterFc;

	if (sf_modEnvToFilterFc /*&& sf_initialFilterFc*/) {
		sp_modEnvToFilterFc = pow(2.0, ((double)sf_modEnvToFilterFc/1200.0));
	}
	else sp_modEnvToFilterFc = 0;

	if (sf_modLfoToFilterFc /* && sf_initialFilterFc*/) {
		sp_modLfoToFilterFc = pow(2.0, ((double)sf_modLfoToFilterFc/1200.0));
	}
	else sp_modLfoToFilterFc = 0;

	if (sf_mod_env_to_pitch) {
		sp_modEnvToPitch = pow(2.0, ((double)sf_mod_env_to_pitch/1200.0));
	}
	else sp_modEnvToPitch = 0;

	sp_cutoff_freq = TO_HZ(val);
	if (val < 0 || val > 24000) val = 19192;
}

/*----------------------------------------------------------------
 * vibrato (LFO2) conversion
 * (note: my changes to Takashi's code are unprincipled --gl)
 *----------------------------------------------------------------*/
#ifndef VIBRATO_RATE_TUNING
#define VIBRATO_RATE_TUNING 38
#endif
static void convert_vibrato(void)
{
	int shift=0, freq=0, delay=0;

	if (sf_delayModLFO) sp_delayModLFO = (int)timecent2msec(sf_delayModLFO);

	if (sf_vibLfoToPitch) {
		shift = sf_vibLfoToPitch;
		if (sf_freqVibLFO) freq = sf_freqVibLFO;
		if (sf_delayVibLFO) delay = (int)timecent2msec(sf_delayVibLFO);
	}
	else if (sf_modLfoToPitch) {
		shift = sf_modLfoToPitch;
		if (sf_freqModLFO) freq = sf_freqModLFO;
		if (sf_delayModLFO) delay = sp_delayModLFO;
	}

	if (!shift) {
		sp_vibrato_depth = sp_vibrato_control_ratio = sp_vibrato_sweep_increment = 0;
		return;
	}

#if 0
	/* cents to linear; 400cents = 256 */
	shift = shift * 256 / 400;
	if (shift > 255) shift = 255;
	else if (shift < -255) shift = -255;
	sp_vibrato_depth = shift;
	/* This is Timidity++ code.  I don't think it makes sense.
	 * vibrato depth seems to be an unsigned 8 bit quantity.
	 */
#else
	/* cents to linear; 400cents = 256 */
	shift = (int)(pow(2.0, ((double)shift/1200.0)) * VIBRATO_RATE_TUNING);
	if (shift < 0) shift = -shift;
	if (shift < 2) shift = 2;
	if (shift > 20) shift = 20; /* arbitrary */
	sp_vibrato_depth = shift;
#endif

	/* frequency in mHz */
	if (!freq) freq = 8;
	else freq = TO_HZ(freq);

	if (freq < 1) freq = 1;

	freq *= 20;
	if (freq > 255) freq = 255;

	/* sp_vibrato_control_ratio = convert_vibrato_rate((unsigned char)freq); */
	sp_vibrato_control_ratio = (unsigned char)freq;

	/* convert mHz to control ratio */
	sp_vibrato_sweep_increment = (unsigned char)(freq/5);
	
	/* sp_vibrato_delay = delay * control_ratio;*/
	sp_vibrato_delay = delay;
}


#ifdef LFO_DEBUG
static void
      convert_lfo(char *name, int program, int banknum, int wanted_bank)
#else
static void
convert_lfo (void)
#endif
{
	int   freq = 0, shift = 0;

	if (!sf_modLfoToFilterFc) {
		sp_lfo_depth = sp_lfo_phase_increment = 0;
		return;
	}

	shift = sf_modLfoToFilterFc;
	if (sf_freqModLFO) freq = sf_freqModLFO;

	shift = (int)(pow(2.0, ((double)shift/1200.0)) * VIBRATO_RATE_TUNING);

	sp_lfo_depth = shift;

	if (!freq) freq = 8 * 20;
	else freq = TO_HZ20(freq);

	if (freq < 1) freq = 1;

	sp_lfo_phase_increment = (short)freq;
#ifdef LFO_DEBUG
	fprintf(stderr,"name=%s, bank=%d(%d), prog=%d, freq=%d\n",
			name, banknum, wanted_bank, program, freq);
#endif
}

/*----------------------------------------------------------------
 * tremolo (LFO1) conversion
 *----------------------------------------------------------------*/

static void convert_tremolo(void)
{
	int level;
	unsigned int freq;

	sp_tremolo_phase_increment = sp_tremolo_sweep_increment = sp_tremolo_depth = 0;

	if (!sf_modLfoToVolume) return;

	level = sf_modLfoToVolume;
	if (level < 0) level = -level;

	level = 255 - (unsigned char)(255 * (1.0 - (level) / (1200.0 * log10(2.0))));

	if (level < 0) level = -level;
	if (level > 20) level = 20; /* arbitrary */
	if (level < 2) level = 2;
	sp_tremolo_depth = level;

	/* frequency in mHz */
	if (!sf_freqModLFO) freq = 8;
	else {
		freq = sf_freqModLFO;
		freq = TO_HZ(freq);
	}

	if (freq < 1) freq = 1;
	freq *= 20;
	if (freq > 255) freq = 255;

	sp_tremolo_phase_increment = (unsigned char)freq;
	sp_tremolo_sweep_increment = ((unsigned char)(freq/5));
}



static int adjust_volume(int start, int length)
{
#if 0
      if (opt_adjust_volume) sp_volume = adjust_volume(sample->dwStart, length);
	 for (i=0; i<length; i++)
	    mem_write16(sf_sample_data[sample->dwStart+i]);
#endif
	    /* Try to determine a volume scaling factor for the sample.
	       This is a very crude adjustment, but things sound more
	       balanced with it. Still, this should be a runtime option. */
	 
	unsigned int  countsamp, numsamps = length;
	unsigned int  higher = 0, highcount = 0;
	short   maxamp = 0, a;
	short  *tmpdta = (short *) sf_sample_data + start;
	double new_vol;
	countsamp = numsamps;
	while (countsamp--) {
		a = *tmpdta++;
		if (a < 0)
		a = -a;
		if (a > maxamp)
		maxamp = a;
	}
	tmpdta = (short *) sf_sample_data + start;
	countsamp = numsamps;
	while (countsamp--) {
		a = *tmpdta++;
		if (a < 0)
		a = -a;
		if (a > 3 * maxamp / 4) {
		    higher += a;
		    highcount++;
		}
	}
	if (highcount)
		higher /= highcount;
	else
		higher = 10000;
	new_vol = (32768.0 * 0.875) / (double) higher;
	return (int)(new_vol * 255.0);
}



/* copies data from the waiting list into a GUS .pat struct */
static int grab_soundfont_sample(char *name, int program, int banknum, int wanted_bank)
{
   sfSample *sample;
   sfGenList *igen;
   sfGenList *pgen;
   sfGenList *global_izone;
   sfGenList *global_pzone;
   float vol, total_vol;
   int igen_count;
   int pgen_count;
   int global_izone_count;
   int global_pzone_count;
   int length;
   int min_freq, max_freq;
   int root_freq;
   int flags;
   int i, n;
   int delay, attack, hold, decay, release, sustain;
   int mod_delay, mod_attack, mod_hold, mod_decay, mod_release, mod_sustain;
   int freq_scale;
   unsigned int sp_volume, sample_volume;

   int debug_examples = 0;


   if (opt_header) {
        VelocityRangeList *vlist;
	int velcount, velcount_part1, k, velmin, velmax, left_patches, right_patches;

	mem_size = 0;

	mem_write_block("GF1PATCH110\0ID#000002\0", 22);

	for (i=0; i<60 && cpyrt[i]; i++) mem_write8(cpyrt[i]);
	for ( ; i<60; i++) mem_write8(0);

	mem_write8(1);                         /* number of instruments */
	mem_write8(14);                        /* number of voices */
	mem_write8(0);                         /* number of channels */
	mem_write16(waiting_list_count);       /* number of waveforms */
	mem_write16(127);                      /* master volume */
	mem_write32(0);                        /* data size (wrong!) */

/* Signal SF2 extensions present */
	mem_write_block("SF2EXT\0", 7);

	/* Remove this -- no longer used. */
/*
	if (wanted_bank == 128) mem_write8(drum_samples_right[banknum][program]);
	else mem_write8(voice_samples_right[banknum][program]);
*/

/* 36 bytes were reserved; now 29 left */
	for (i=8; i<37; i++)                   /* reserved */
	   mem_write8(0);

	mem_write16(0);                        /* instrument number */

	for (i=0; name[i] && i < 16; i++)      /* instrument name */
	   mem_write8(name[i]);

	while (i < 16) {                       /* pad instrument name */
	   mem_write8(0);
	   i++;
	}

	mem_write32(0);                        /* instrument size (wrong!) */
	mem_write8(1);                         /* number of layers */


    /* List of velocity layers with left and right patch counts. There is room for 10 here.
     * For each layer, give four bytes: velocity min, velocity max, #left patches, #right patches.
     */
	if (wanted_bank == 128 || opt_drum) vlist = drum_velocity[banknum][program];
	else vlist = voice_velocity[banknum][program];
	if (vlist) velcount = vlist->range_count;
	else velcount = 1;
	if (opt_small) velcount = 1;
	if (velcount > 19) velcount = 19;
	if (velcount > 9) velcount_part1 = 9;
	else velcount_part1 = velcount;

	mem_write8(velcount);

	for (k = 0; k < velcount_part1; k++) {
		if (vlist) {
			velmin = vlist->velmin[k];
			velmax = vlist->velmax[k];
			left_patches = vlist->left_patches[k] + vlist->mono_patches[k];
			if (vlist->right_patches[k] && !opt_mono)
				right_patches = vlist->right_patches[k]/* + vlist->mono_patches[k]*/;
			else right_patches = 0;
		}
		else {
			fprintf(stderr,"Internal error.\n");
			exit(1);
		}

		mem_write8(velmin);
		mem_write8(velmax);
		mem_write8(left_patches);
		mem_write8(right_patches);
	}

	for (i=0; i<40-1-4*velcount_part1; i++)                   /* reserved */
	   mem_write8(0);


	mem_write8(0);                         /* layer duplicate */
	mem_write8(0);                         /* layer number */
	mem_write32(0);                        /* layer size (wrong!) */
	mem_write8(waiting_list_count);        /* number of samples */

	if (velcount > velcount_part1) {
	   for (k = velcount_part1; k < velcount; k++) {
		if (vlist) {
			velmin = vlist->velmin[k];
			velmax = vlist->velmax[k];
			left_patches = vlist->left_patches[k] + vlist->mono_patches[k];
			if (vlist->right_patches[k] && !opt_mono)
				right_patches = vlist->right_patches[k]/* + vlist->mono_patches[k]*/;
			else right_patches = 0;
		}
		else {
			fprintf(stderr,"Internal error.\n");
			exit(1);
		}

		mem_write8(velmin);
		mem_write8(velmax);
		mem_write8(left_patches);
		mem_write8(right_patches);
	   }
	   for (i=0; i<40-4*(velcount-velcount_part1); i++)                   /* reserved */
	      mem_write8(0);
	}
	else {
	   for (i=0; i<40; i++)                   /* reserved */
	      mem_write8(0);
	}
   }
   /* bodge alert!!! I don't know how to make the volume parameters come
    * out right. If I ignore them, some things are very wrong (eg. with
    * the Emu 2MB bank the pads have overloud 'tinkle' layers, and the
    * square lead is way too loud). But if I process the attenuation
    * parameter the way I think it should be done, other things go wrong
    * (the church organ and honkytonk piano come out very quiet). So I
    * look at the volume setting and then normalise the results, to get
    * the differential between each layer but still keep a uniform overall
    * volume for the patch. Totally incorrect, but it usually seems to do 
    * the right thing...
    *
    * (This is currently disabled for 16 bit samples. gl)
    */

   total_vol = 0;

   for (n=0; n<waiting_list_count; n++) {
      int v = 0;
      int keymin = 0;
      int keymax = 127;

      /* look for volume and keyrange generators */
      for (i=0; i<waiting_list[n].igen_count; i++) {
	 if (waiting_list[n].igen[i].sfGenOper == SFGEN_initialAttenuation) {
	    v = waiting_list[n].igen[i].genAmount.shAmount;
	 }
	 else if (waiting_list[n].igen[i].sfGenOper == SFGEN_keyRange) {
	    keymin = waiting_list[n].igen[i].genAmount.ranges.byLo;
	    keymax = waiting_list[n].igen[i].genAmount.ranges.byHi;
	 }
      }

      for (i=0; i<waiting_list[n].pgen_count; i++) {
	 if (waiting_list[n].pgen[i].sfGenOper == SFGEN_initialAttenuation) {
	    v += waiting_list[n].pgen[i].genAmount.shAmount;
	 }
	 else if (waiting_list[n].pgen[i].sfGenOper == SFGEN_keyRange) {
	    keymin = waiting_list[n].pgen[i].genAmount.ranges.byLo;
	    keymax = waiting_list[n].pgen[i].genAmount.ranges.byHi;
	 }
      }


      /* convert centibels to scaling factor (I _think_ this is right :-) */
      vol = 1.0;
      while (v-- > 0)
	 vol /= pow(10, 0.005);

      waiting_list[n].volume = vol;

      if ((keymin <= 60) && (keymax >= 60))
	 total_vol += vol;
   }

   /* This normalization does not seem to work well for
    * timidity -- perhaps because it does it's own normalization.
    * So I don't use this value for the case of 16 bit samples,
    * though I've left it in place for 8 bit samples.  Perhaps
    * unwisely. (gl)
    */
   /* normalise the layer volumes so they sum to unity */
   if (total_vol > 0) {
      for (n=0; n<waiting_list_count; n++)
	 waiting_list[n].volume = MID(0.2, waiting_list[n].volume/total_vol, 1.0);
   }

   /* for each sample... */
   for (n=0; n<waiting_list_count; n++) {
      sample = waiting_list[n].sample;
      igen = waiting_list[n].igen;
      pgen = waiting_list[n].pgen;
      global_izone = waiting_list[n].global_izone;
      global_pzone = waiting_list[n].global_pzone;
      igen_count = waiting_list[n].igen_count;
      pgen_count = waiting_list[n].pgen_count;
      global_izone_count = waiting_list[n].global_izone_count;
      global_pzone_count = waiting_list[n].global_pzone_count;
      vol = waiting_list[n].volume;

      /* set default generator values */
      sf_instrument_look_index = -1; /* index into INST subchunk */
      sf_sample_look_index = -1;

      sf_start = sample->dwStart;
      sf_end = sample->dwEnd;
      sf_loop_start = sample->dwStartloop;
      sf_loop_end = sample->dwEndloop;
      sf_key = sample->byOriginalKey;
      sf_tune = sample->chCorrection;
      sf_sustain_mod_env = 0;
      sf_mod_env_to_pitch = 0;

      sf_delay_vol_env = -12000;
      sf_attack_vol_env = -12000;
      sf_hold_vol_env = -12000;
      sf_decay_vol_env = -12000;
      sf_release_vol_env = -12000;
      sf_sustain_level = 250;

      sf_delayModEnv = -12000;
      sf_attackModEnv = -12000;
      sf_holdModEnv = -12000;
      sf_decayModEnv = -12000;
      sf_releaseModEnv = -12000;

      sf_pan = 0;
      sf_keyscale = 100;
      sf_keymin = 0;
      sf_keymax = 127;
      sf_velmin = 0;
      sf_velmax = 127;
      sf_mode = 0;
      /* I added the following. (gl) */
      sf_instrument_unused5 = -1;
      sf_exclusiveClass = 0;
      sf_initialAttenuation = 0;
      sf_chorusEffectsSend = 0;
      sf_reverbEffectsSend = 0;
      sf_modLfoToPitch = 0;
      sf_vibLfoToPitch = 0;
      sf_keynum = -1;
      sf_velocity = -1;
      sf_keynumToModEnvHold = 0;
      sf_keynumToModEnvDecay = 0;
      sf_keynumToVolEnvHold = 0;
      sf_keynumToVolEnvDecay = 0;
      sf_modLfoToVolume = 0;
      sf_delayModLFO = 0;
      sf_freqModLFO = 0;
      sf_delayVibLFO = 0;
      sf_freqVibLFO = 0;
      sf_initialFilterQ = 0;
      sf_initialFilterFc = 0;
      sf_modEnvToFilterFc = 0;
      sf_modLfoToFilterFc = 0;

      sp_freq_center = 60;

      /* process the lists of generator data */
      for (i=0; i<global_izone_count; i++)
	 apply_generator(&global_izone[i], FALSE, TRUE);

      for (i=0; i<igen_count; i++)
	 apply_generator(&igen[i], FALSE, FALSE);

      for (i=0; i<global_pzone_count; i++)
	 apply_generator(&global_pzone[i], TRUE, TRUE);

      for (i=0; i<pgen_count; i++)
	 apply_generator(&pgen[i], TRUE, FALSE);

      /* convert SoundFont values into some more useful formats */
      length = sf_end - sf_start;

      if (length < 0) {
	      fprintf(stderr,"\nSample for %s has negative length.\n", name);
	      return FALSE;
      }
      sf_loop_start = MID(0, sf_loop_start-sf_start, sf_end);
      sf_loop_end = MID(0, sf_loop_end-sf_start, sf_end);

      /*sf_pan = MID(0, sf_pan*16/1000+7, 15);*/
      sf_pan = MID(0, sf_pan*256/1000+127, 255);

      if (sf_keyscale == 100) freq_scale = 1024;
      else freq_scale = MID(0, sf_keyscale*1024/100, 2048);

/* I don't know about this tuning. (gl) */
      //sf_tune += sf_mod_env_to_pitch * MID(0, 1000-sf_sustain_mod_env, 1000) / 1000;

      min_freq = freq_table[sf_keymin];
      max_freq = freq_table[sf_keymax];

      root_freq = calc_root_pitch();

      sustain = calc_sustain();
      sp_volume = calc_volume();

      if (sustain < 0) sustain = 0;
      if (sustain > sp_volume - 2) sustain = sp_volume - 2;

/*
	if (!lay->set[SF_releaseEnv2] && banknum < 128) release = 400;
	if (!lay->set[SF_decayEnv2] && banknum < 128) decay = 400;
*/
      delay = timecent2msec(sf_delay_vol_env);
      attack = timecent2msec(sf_attack_vol_env);
      hold = timecent2msec(sf_hold_vol_env);
      decay = timecent2msec(sf_decay_vol_env);
      release = timecent2msec(sf_release_vol_env);

      mod_sustain = calc_mod_sustain();
      mod_delay = timecent2msec(sf_delayModEnv);
      mod_attack = timecent2msec(sf_attackModEnv);
      mod_hold = timecent2msec(sf_holdModEnv);
      mod_decay = timecent2msec(sf_decayModEnv);
      mod_release = timecent2msec(sf_releaseModEnv);

      /* The output from this code is almost certainly not a 'correct'
       * .pat file. There are a lot of things I don't know about the
       * format, which have been filled in by guesses or values copied
       * from the Gravis files. And I have no idea what I'm supposed to
       * put in all the size fields, which are currently set to zero :-)
       *
       * But, the results are good enough for DIGMID to understand, and
       * the CONVERT program also seems quite happy to accept them, so
       * it is at least mostly correct...
       */
      sample = waiting_list[n].sample;

      mem_write8('s');                    /* sample name */
      mem_write8('m');
      mem_write8('p');
      mem_write8('0'+(n+1)/10);
      mem_write8('0'+(n+1)%10);
      if (waiting_list[n].stereo_mode == LEFT_SAMPLE)
	      mem_write8('L');
      else if (waiting_list[n].stereo_mode == RIGHT_SAMPLE)
	      mem_write8('R');
      else if (waiting_list[n].stereo_mode == MONO_SAMPLE)
	      mem_write8('M');
      else mem_write8('0'+waiting_list[n].stereo_mode);
      mem_write8(0);


      mem_write8(0);                      /* fractions */

      if (opt_8bit) {
	 mem_write32(length);             /* waveform size */
	 mem_write32(sf_loop_start);      /* loop start */
	 mem_write32(sf_loop_end);        /* loop end */
      }
      else {
	 mem_write32(length*2);           /* waveform size */
	 mem_write32(sf_loop_start*2);    /* loop start */
	 mem_write32(sf_loop_end*2);      /* loop end */
      }

      mem_write16(sample->dwSampleRate);  /* sample freq */

      mem_write32(min_freq);              /* low freq */
      mem_write32(max_freq);              /* high freq */
      mem_write32(root_freq);             /* root frequency */

      mem_write16(512);                   /* finetune */
      /*mem_write8(sf_pan);*/                 /* balance */
      mem_write8(7);	                 /* balance = middle */

/* if (wanted_bank != 128 && banknum == 0 && program == 56) debug_examples = -1; */
if (debug_examples < 0) {
	debug_examples++;
      printf("attack_vol_env=%d, hold_vol_env=%d, decay_vol_env=%d, release_vol_env=%d, sf_delay=%d\n",
		      sf_attack_vol_env, sf_hold_vol_env, sf_decay_vol_env, sf_release_vol_env,
		      sf_delay_vol_env);
	printf("iA= %d, sp_volume=%d sustain=%d attack=%d ATTACK=%d\n", sf_initialAttenuation,
		       	sp_volume, sustain, attack, msec2gus(attack, sp_volume));
	printf("\thold=%d r=%d HOLD=%d\n", hold, sp_volume-1, msec2gus(hold, sp_volume-1));
	printf("\tdecay=%d r=%d DECAY=%d\n", hold, sp_volume-1-sustain, msec2gus(hold, sp_volume-1-sustain));
	printf("\trelease=%d r=255 RELEASE=%d\n", release, msec2gus(release, 255));
	printf("  levels: %d %d %d %d\n", sp_volume, sp_volume-1, sustain, 0);
}

      mem_write8(msec2gus(attack, sp_volume));                   /* envelope rates */
      mem_write8(msec2gus(hold, sp_volume - 1));
      mem_write8(msec2gus(decay, sp_volume - 1 - sustain));
      mem_write8(msec2gus(release, 255));
      mem_write8(0x3F);
      mem_write8(0x3F);

      mem_write8(sp_volume);                    /* envelope offsets */
      mem_write8(sp_volume - 1);
      mem_write8(sustain);
      mem_write8(0);
      mem_write8(0);
      mem_write8(0);

      convert_tremolo();
      mem_write8(sp_tremolo_sweep_increment);    /* tremolo sweep */
      mem_write8(sp_tremolo_phase_increment);    /* tremolo rate */
      mem_write8(sp_tremolo_depth);              /* tremolo depth */

      convert_vibrato();
      mem_write8(sp_vibrato_sweep_increment);     /* vibrato sweep */
      mem_write8(sp_vibrato_control_ratio);      /* vibrato rate */
      mem_write8(sp_vibrato_depth);               /* vibrato depth */

#ifdef LFO_DEBUG
      convert_lfo(name, program, banknum, wanted_bank);
#else
      convert_lfo();
#endif

      flags = getmodes(sf_mode, program, wanted_bank);

      mem_write8(flags);                  /* write sample mode */

      /* The value for sp_freq_center was set in calc_root_pitch(). */
      mem_write16(sp_freq_center);
      mem_write16(freq_scale);           /* scale factor */

      if (opt_adjust_volume) {
	  if (opt_veryverbose) printf("vol comp %d", sp_volume);
          sample_volume = adjust_volume(sample->dwStart, length);
	  if (opt_veryverbose) printf(" -> %d\n", sample_volume);
      }
      else sample_volume = sp_volume;

      mem_write16(sample_volume); /* I'm not sure this is here. (gl) */

/* Begin SF2 extensions */
      mem_write8(delay);
      mem_write8(sf_exclusiveClass);
      mem_write8(sp_vibrato_delay);

      mem_write8(msec2gus(mod_attack, sp_volume));                   /* envelope rates */
      mem_write8(msec2gus(mod_hold, sp_volume - 1));
      mem_write8(msec2gus(mod_decay, sp_volume - 1 - mod_sustain));
      mem_write8(msec2gus(mod_release, 255));
      mem_write8(0x3F);
      mem_write8(0x3F);

      mem_write8(sp_volume);                    /* envelope offsets */
      mem_write8(sp_volume - 1);
      mem_write8(mod_sustain);
      mem_write8(0);
      mem_write8(0);
      mem_write8(0);

      mem_write8(sp_delayModLFO);

      mem_write8(sf_chorusEffectsSend);
      mem_write8(sf_reverbEffectsSend);

      calc_resonance();
      mem_write16(sp_resonance);

      calc_cutoff();
      mem_write16(sp_cutoff_freq);
/*
fprintf(stderr,"b=%2d p=%3d wb=%2d cutoff=%5d res=%3d\n", banknum,
		program, wanted_bank, sp_cutoff_freq, sp_resonance);
*/
      mem_write8(sp_modEnvToPitch);
      mem_write8(sp_modEnvToFilterFc);
      mem_write8(sp_modLfoToFilterFc);

      mem_write8(sf_keynumToModEnvHold);
      mem_write8(sf_keynumToModEnvDecay);
      mem_write8(sf_keynumToVolEnvHold);
      mem_write8(sf_keynumToVolEnvDecay);

      mem_write8(sf_pan);                 /* balance */

      mem_write16(sp_lfo_phase_increment);    /* lfo */
      mem_write8(sp_lfo_depth);

      if (sf_instrument_unused5 == -1)
	mem_write8(255);
      else mem_write8(sf_instrument_unused5);

#if 0
/* 36 (34?) bytes were reserved; now 1 left */
      for (i=35; i<36; i++)                /* reserved */
	 mem_write8(0);
#endif
      if (opt_8bit) {                     /* sample waveform */
	 for (i=0; i<length; i++)
	    mem_write8((int)((sf_sample_data[sample->dwStart+i] >> 8) * vol) ^ 0x80);
      }
      else {
	 for (i=0; i<length; i++)
	    mem_write16(sf_sample_data[sample->dwStart+i]);
      }
   }
   return TRUE;
}



/* converts loaded SoundFont data */
static int grab_soundfont(int num, int drum, char *name)
{
   sfPresetHeader *pheader;
   sfPresetBag *pindex;
   sfGenList *pgen;
   sfInst *iheader;
   sfInstBag *iindex;
   sfGenList *igen;
   sfGenList *global_izone;
   sfGenList *global_pzone;
   sfSample *sample;
   int pindex_count;
   int pgen_count;
   int iindex_count;
   int igen_count;
   int pnum, inum, jnum, lnum;
   int global_izone_index;
   int global_izone_count;
   int global_pzone_index;
   int global_pzone_count;
   int wanted_patch, wanted_bank;
   int keymin, keymax;
   int wanted_keymin, wanted_keymax;
   int velmin, velmax;
   int waiting_room_full;
   int i;
   char *s;

   if (drum) {
      if (opt_drum) {
	 wanted_patch = opt_drum_bank;
      	 wanted_bank = 0;
      }
      else {
          wanted_patch = opt_drum_bank;
          wanted_bank = 128;
      }
      wanted_keymin = num;
      wanted_keymax = num;
   }
   else {
      wanted_patch = num;
      wanted_bank = opt_bank;
      wanted_keymin = 0;
      wanted_keymax = 127;
   }

   /* search for the desired preset */
   for (pnum=0; pnum<sf_num_presets; pnum++) {
      int global_preset_layer, global_preset_velmin, global_preset_velmax, preset_velmin, preset_velmax;
      int global_preset_keymin, global_preset_keymax, preset_keymin, preset_keymax;

      pheader = &sf_presets[pnum];

      if ((pheader->wPreset == wanted_patch) && (pheader->wBank == wanted_bank)) {
	 /* find what substructures it uses */
	 pindex = &sf_preset_indexes[pheader->wPresetBagNdx];
	 pindex_count = pheader[1].wPresetBagNdx - pheader[0].wPresetBagNdx;

	 if (pindex_count < 1)
	    return FALSE;

	 /* prettify the preset name */
	 s = pheader->achPresetName;

	 i = strlen(s)-1;
	 while ((i >= 0) && (isspace(s[i]))) {
	    s[i] = 0;
	    i--;
	 }

	 if (opt_verbose) printf("Grabbing %s%s -> %s\n", opt_right_channel? "R ":"L ", s, name);
	 else if (!opt_no_write) { printf("."); fflush(stdout); }

	 waiting_list_count = 0;
	 waiting_room_full = FALSE;

	 global_pzone = NULL;
	 global_pzone_index = -1;
	 global_pzone_count = 0;

	 global_preset_velmin = preset_velmin = -1;
	 global_preset_velmax = preset_velmax = -1;
	 global_preset_keymin = preset_keymin = -1;
	 global_preset_keymax = preset_keymax = -1;

	 /* for each layer in this preset */
	 for (inum=0; inum<pindex_count; inum++) {
	    int global_instrument_layer, global_instrument_velmin, global_instrument_velmax,
	    	instrument_velmin, instrument_velmax;
	    int global_instrument_keymin, global_instrument_keymax,
	    	instrument_keymin, instrument_keymax;

	    pgen = &sf_preset_generators[pindex[inum].wGenNdx];
	    pgen_count = pindex[inum+1].wGenNdx - pindex[inum].wGenNdx;

	    if (pgen_count < 0) break;
	    //velmin = 0;
	    //velmax = 127;

	    if (global_preset_velmin >= 0) preset_velmin = global_preset_velmin;
	    if (global_preset_velmax >= 0) preset_velmax = global_preset_velmax;
	    if (global_preset_keymin >= 0) preset_keymin = global_preset_keymin;
	    if (global_preset_keymax >= 0) preset_keymax = global_preset_keymax;

	    if (pgen_count > 0 && pgen[pgen_count-1].sfGenOper != SFGEN_instrument) { /* global preset zone */
		     global_pzone = pgen;
		     global_pzone_index = inum;
		     global_pzone_count = pgen_count;
		     global_preset_layer = TRUE;
	    }
	    else global_preset_layer = FALSE;

	    if (pgen[0].sfGenOper == SFGEN_keyRange) {
		preset_keymin = pgen[0].genAmount.ranges.byLo;
		preset_keymax = pgen[0].genAmount.ranges.byHi;
		if (global_preset_layer) {
			global_preset_keymin = preset_keymin;
			global_preset_keymax = preset_keymax;
		}
	    }

	    for (jnum=0; jnum<pgen_count; jnum++) {
	     if (pgen[jnum].sfGenOper == SFGEN_velRange) {
		preset_velmin = pgen[jnum].genAmount.ranges.byLo;
		preset_velmax = pgen[jnum].genAmount.ranges.byHi;
		if (global_preset_layer) {
			global_preset_velmin = preset_velmin;
			global_preset_velmax = preset_velmax;
		}
	     }
	    }

	    /* find what instrument we should use */
	    if ((pgen_count > 0) && 
		(pgen[pgen_count-1].sfGenOper == SFGEN_instrument)) {

#if 0
	       if (pgen[0].sfGenOper == SFGEN_keyRange)
		  if ((pgen[0].genAmount.ranges.byHi < keymin) ||
		      (pgen[0].genAmount.ranges.byLo > keymax))
		     continue;
#endif

	       iheader = &sf_instruments[pgen[pgen_count-1].genAmount.wAmount];

	       iindex = &sf_instrument_indexes[iheader->wInstBagNdx];
	       iindex_count = iheader[1].wInstBagNdx - iheader[0].wInstBagNdx;

	       global_instrument_velmin = instrument_velmin = -1;
	       global_instrument_velmax = instrument_velmax = -1;
	       global_instrument_keymin = instrument_keymin = -1;
	       global_instrument_keymax = instrument_keymax = -1;


	       global_izone = NULL;
	       global_izone_index = -1;
	       global_izone_count = 0;

	       /* for each layer in this instrument */
	       for (lnum=0; lnum<iindex_count; lnum++) {
		  igen = &sf_instrument_generators[iindex[lnum].wInstGenNdx];
		  igen_count = iindex[lnum+1].wInstGenNdx - iindex[lnum].wInstGenNdx;

		  if (global_instrument_velmin >= 0) instrument_velmin = global_instrument_velmin;
		  if (global_instrument_velmax >= 0) instrument_velmax = global_instrument_velmax;
		  if (global_instrument_keymin >= 0) instrument_keymin = global_instrument_keymin;
		  if (global_instrument_keymax >= 0) instrument_keymax = global_instrument_keymax;

		  if ((igen_count > 0) && 
		      (igen[igen_count-1].sfGenOper != SFGEN_sampleID))
			  global_instrument_layer = TRUE;
		  else global_instrument_layer = FALSE;

		  for (jnum=0; jnum<igen_count; jnum++) {
		     if (igen[jnum].sfGenOper == SFGEN_velRange) {
			instrument_velmin = igen[jnum].genAmount.ranges.byLo;
			instrument_velmax = igen[jnum].genAmount.ranges.byHi;
			if (global_instrument_layer) {
				global_instrument_velmin = instrument_velmin;
				global_instrument_velmax = instrument_velmax;
			}
		     }
		  }

		  if (igen_count > 0 && igen[0].sfGenOper == SFGEN_keyRange) {
			instrument_keymin = igen[0].genAmount.ranges.byLo;
			instrument_keymax = igen[0].genAmount.ranges.byHi;
			if (global_instrument_layer) {
				global_instrument_keymin = instrument_keymin;
				global_instrument_keymax = instrument_keymax;
			}
		  }

		  if (instrument_velmin >= 0) velmin = instrument_velmin;
		  else velmin = 0;
		  if (instrument_velmax >= 0) velmax = instrument_velmax;
		  else velmax = 127;
		  if (preset_velmin >= 0 && preset_velmax >= 0) {
			  if (preset_velmin >= velmin && preset_velmax <= velmax) {
				  velmin = preset_velmin;
				  velmax = preset_velmax;
			  }
		  }

		  if (instrument_keymin >= 0) keymin = instrument_keymin;
		  else keymin = 0;
		  if (instrument_keymax >= 0) keymax = instrument_keymax;
		  else keymax = 127;
		  if (preset_keymin >= 0 && preset_keymax >= 0) {
			  if (preset_keymin >= keymin && preset_keymax <= keymax) {
				  keymin = preset_keymin;
				  keymax = preset_keymax;
			  }
		  }
#if 0
if (!drum && !wanted_bank && wanted_patch == 16) {
	fprintf(stderr,"velmin=%d velmax=%d wanted_velmin=%d wanted_velmax=%d\n",
			velmin, velmax, wanted_velmin, wanted_velmax);
	fprintf(stderr,"instrument_velmin=%d instrument_velmax=%d preset_velmin=%d preset_velmax=%d\n",
			instrument_velmin, instrument_velmax, preset_velmin, preset_velmax);
	fprintf(stderr,"keymin=%d keymax=%d wanted_keymin=%d wanted_keymax=%d\n",
			keymin, keymax, wanted_keymin, wanted_keymax);
	fprintf(stderr,"instrument_keymin=%d instrument_keymax=%d preset_keymin=%d preset_keymax=%d\n",
			instrument_keymin, instrument_keymax, preset_keymin, preset_keymax);
}
#endif
		  if (velmin != wanted_velmin || velmax != wanted_velmax) continue;
		  if (drum && (wanted_keymin < keymin || wanted_keymin > keymax)) continue;
		  if (!drum && (keymin < wanted_keymin || keymax > wanted_keymax)) continue;

		  /* find what sample we should use */
		  if ((igen_count > 0) && 
		      (igen[igen_count-1].sfGenOper == SFGEN_sampleID)) {

		     sample = &sf_samples[igen[igen_count-1].genAmount.wAmount];

		     /* sample->wSampleLink is the link?? */
		     /* lsample = &sf_samples[sample->wSampleLink] */


		     if (sample->sfSampleType & LINKED_SAMPLE) continue; /* linked */

		     s = sample->achSampleName;

		     i = strlen(s)-1;

		     if (s[i] == 'L') sample->sfSampleType = LEFT_SAMPLE;
		     if (s[i] == 'R') sample->sfSampleType = RIGHT_SAMPLE;

		     if (sample->sfSampleType == LEFT_SAMPLE && !opt_left_channel) continue;
		     if (sample->sfSampleType == RIGHT_SAMPLE && !opt_right_channel) continue;
		     if (sample->sfSampleType == MONO_SAMPLE && opt_right_channel) continue;

		     /* prettify the sample name */

		     if (opt_verbose) {
			     int j = i - 3;
			     if (j < 0) j = 0;
			     while (j <= i) {
				     if (s[j] == 'R') break;
				     if (s[j] == 'L' && j < i && s[j+1] == 'o') { j++; continue; }
				     if (s[j] == 'L') break;
				     j++;
			     }
			     if (j <= i) {
				     if (s[j] == 'R' && sample->sfSampleType != RIGHT_SAMPLE)
					     printf("Note that sample name %s is not a right sample\n", s);
				     //if (s[j] == 'R' && sample->sfSampleType == RIGHT_SAMPLE)
				     //	     printf("OK -- sample name %s is a right sample\n", s);
				     if (s[j] == 'L' && sample->sfSampleType != LEFT_SAMPLE)
					     printf("Note that sample name %s is not a left sample\n", s);
				     //if (s[j] == 'L' && sample->sfSampleType == LEFT_SAMPLE)
				     //	     printf("OK -- sample name %s is a left sample\n", s);
			     }
		     }

		     while ((i >= 0) && (isspace(s[i]))) {
			s[i] = 0;
			i--;
		     }

		     if (sample->sfSampleType & 0x8000) {
			printf("\nThis SoundFont uses AWE32 ROM data in sample %s\n", s);
			if (opt_veryverbose)
			   printf("\n");
			return FALSE;
		     }

		     /* add this sample to the waiting list */
		     if (waiting_list_count < MAX_WAITING) {
			if (opt_veryverbose)
			   printf("  - sample %s\n", s);

			waiting_list[waiting_list_count].sample = sample;
			waiting_list[waiting_list_count].igen = igen;
			waiting_list[waiting_list_count].pgen = pgen;
			waiting_list[waiting_list_count].global_izone = global_izone;
			waiting_list[waiting_list_count].global_pzone = global_pzone;
			waiting_list[waiting_list_count].igen_count = igen_count;
			waiting_list[waiting_list_count].pgen_count = pgen_count;
			waiting_list[waiting_list_count].global_izone_count = global_izone_count;
			waiting_list[waiting_list_count].global_pzone_count = global_pzone_count;
			waiting_list[waiting_list_count].volume = 1.0;
			waiting_list[waiting_list_count].stereo_mode = sample->sfSampleType;
			waiting_list_count++;

		     }
		     else 
			waiting_room_full = TRUE;
		  }
		  else if (igen_count > 0) { /* global instrument zone */

		     global_izone = igen;
		     global_izone_index = lnum;
		     global_izone_count = igen_count;
		  }
	       }
	    }
	 }

	 if (waiting_room_full)
	    printf("Warning: too many layers in this instrument!\n");

	 if (waiting_list_count > 0) {
	    int pcount, vcount, k;
	    VelocityRangeList *vlist;
	    if (drum) vlist = drum_velocity[wanted_patch][wanted_keymin];
	    else vlist = voice_velocity[wanted_bank][wanted_patch];
	    if (!vlist) {
		    fprintf(stderr,"\nNo record found for %s, keymin=%d patch=%d bank=%d\n",
				    name, wanted_keymin, wanted_patch, wanted_bank);
		    return FALSE;
	    }
	    vcount = vlist->range_count;
	    for (k = 0; k < vcount; k++)
		    if (vlist->velmin[k] == wanted_velmin && vlist->velmax[k] == wanted_velmax) break;
	    if (k == vcount) {
		    fprintf(stderr,"\n%s patches were requested for an unknown velocity range.\n", name);
		    return FALSE;
	    }
	    if (opt_right_channel) pcount = vlist->right_patches[k];
	    else pcount = vlist->left_patches[k] + vlist->mono_patches[k];
#if 0
	    if (opt_header && pcount > waiting_list_count && opt_left_channel) {
		    while (pcount > waiting_list_count && vlist->right_patches[k] > 1 && vlist->left_patches[k] > 1) {
			    vlist->right_patches[k]--;
			    vlist->left_patches[k]--;
			    pcount--;
		    }
		    while (pcount > waiting_list_count && vlist->mono_patches[k] > 1) {
			    vlist->mono_patches[k]--;
			    pcount--;
		    }
	    }
#endif
	    if (pcount != waiting_list_count) {
		fprintf(stderr,"\nFor %sinstrument %s %s found %d samples when there should be %d samples.\n",
				    opt_header? "header of ": "", name,
				    opt_left_channel? "left/mono": "right",
				    waiting_list_count, pcount);
		fprintf(stderr,"\tkeymin=%d keymax=%d patch=%d bank=%d, velmin=%d, velmax=%d\n",
				    wanted_keymin, wanted_keymax, wanted_patch, wanted_bank,
				    wanted_velmin, wanted_velmax);
		fprintf(stderr,"\tleft patches %d, right patches %d, mono patches %d\n",
				    vlist->left_patches[k],
				    vlist->right_patches[k],
				    vlist->mono_patches[k]);
		return FALSE;
	    }
	    if (opt_verbose && vlist->other_patches[k]) {
		fprintf(stderr,"\nFor instrument %s found %d samples in unknown channel.\n",
				name, vlist->other_patches[k]);
	    }
	    if (drum) return grab_soundfont_sample(name, wanted_keymin, wanted_patch, wanted_bank);
	    else return grab_soundfont_sample(name, wanted_patch, wanted_bank, wanted_bank);
	 }
	 else {
	    fprintf(stderr,"\nStrange... no valid layers found in instrument %s bank %d prog %d\n",
			    name, drum? wanted_patch:wanted_bank, drum? wanted_keymin:wanted_patch);
		return FALSE;
	 }
	 if (opt_veryverbose)
	    printf("\n");

	 return (waiting_list_count > 0);
      }
   }

   return FALSE;
}

static void record_velocity_range(int drum, int banknum, int program,
	       	int velmin, int velmax, int type) {
	int i, count;
	VelocityRangeList *vlist;
	char *name;

	if (drum) {
		vlist = drum_velocity[banknum][program];
		name = drum_name[banknum][program];
	}
	else {
		vlist = voice_velocity[banknum][program];
		name = voice_name[banknum][program];
	}

	if (!vlist) {
		vlist = (VelocityRangeList *)malloc(sizeof(VelocityRangeList));
		if (drum) drum_velocity[banknum][program] = vlist;
		else voice_velocity[banknum][program] = vlist;
		vlist->range_count = 0;
	}
	count = vlist->range_count;
	for (i = 0; i < count; i++) {
		if (vlist->velmin[i] == velmin && vlist->velmax[i] == velmax) break;
	}
	if (i >= 128) return;
	vlist->velmin[i] = velmin;
	vlist->velmax[i] = velmax;
	if (i == count) {
		vlist->mono_patches[i] = 0;
		vlist->left_patches[i] = 0;
		vlist->right_patches[i] = 0;
		vlist->other_patches[i] = 0;
		vlist->range_count++;
	}
	if (type == RIGHT_SAMPLE) vlist->right_patches[i]++;
	else if (type == LEFT_SAMPLE) vlist->left_patches[i]++;
	else if (type == MONO_SAMPLE) vlist->mono_patches[i]++;
	else {
		vlist->mono_patches[i]++;
		vlist->other_patches[i]++;
	}

if (opt_veryverbose)
	printf("%s#%d velocity range %d-%d for %s chan %s has %d patches\n", (i==count)? "new ":"",
		vlist->range_count, velmin, velmax,
	       	name, (type==LEFT_SAMPLE)? "left" : (type==RIGHT_SAMPLE)? "right" : "mono",
		(type==RIGHT_SAMPLE)? vlist->right_patches[i] : (type==LEFT_SAMPLE)? vlist->left_patches[i] :
		vlist->mono_patches[i]);
}

/* gets facts and names */
static int grab_soundfont_banks(void)
{
   sfPresetHeader *pheader;
   sfPresetBag *pindex;
   sfGenList *pgen;
   sfInst *iheader;
   sfInstBag *iindex;
   sfGenList *igen;
   sfSample *sample;
   int pindex_count;
   int pgen_count;
   int iindex_count;
   int igen_count;
   int pnum, inum, jnum, lnum, num, drum;
   int wanted_patch, wanted_bank;
   int keymin, keymax, drumnum;
   int velmin, velmax;
   int i, j;
   char *s;
   char tmpname[80];

   for (i = 0; i < 128; i++) {
	   tonebank[i] = FALSE;
	   drumset_name[i] = NULL;
	   drumset_short_name[i] = NULL;
	   for (j = 0; j < 128; j++) {
		   voice_name[i][j] = NULL;
		   voice_samples_mono[i][j] = 0;
		   voice_samples_left[i][j] = 0;
		   voice_samples_right[i][j] = 0;
		   voice_velocity[i][j] = NULL;
		   drum_name[i][j] = NULL;
		   drum_samples_mono[i][j] = 0;
		   drum_samples_left[i][j] = 0;
		   drum_samples_right[i][j] = 0;
		   drum_velocity[i][j] = NULL;
	   }
   }

   keymin = 0;
   keymax = 127;

   /* search for the desired preset */
   for (pnum=0; pnum<sf_num_presets; pnum++) {
	 int global_preset_layer, global_preset_velmin, global_preset_velmax, preset_velmin, preset_velmax;
	 int global_preset_keymin, global_preset_keymax, preset_keymin, preset_keymax;

	 pheader = &sf_presets[pnum];


	 wanted_patch = pheader->wPreset;
	 wanted_bank = pheader->wBank;

	 if (wanted_bank == 128 || opt_drum) {
		 drum = TRUE;
		 opt_drum_bank = wanted_patch;
	 }
	 else {
		 drum = FALSE;
		 opt_bank = wanted_bank;
	 }

	 /* find what substructures it uses */
	 pindex = &sf_preset_indexes[pheader->wPresetBagNdx];
	 pindex_count = pheader[1].wPresetBagNdx - pheader[0].wPresetBagNdx;

	 if (pindex_count < 1)
	    continue;

	 /* prettify the preset name */
	 s = getname(pheader->achPresetName);

	 if (drum) {
		 if (!drumset_name[opt_drum_bank]) {
			drumset_short_name[opt_drum_bank] = strdup(s);
			sprintf(tmpname, "%s-%s", basename, s);
		 	drumset_name[opt_drum_bank] = strdup(tmpname);
		 	if (opt_verbose) printf("drumset #%d %s\n", opt_drum_bank, s);
		 }
	 }
	 else {
		 if (!voice_name[opt_bank][wanted_patch]) {
		 	voice_name[opt_bank][wanted_patch] = strdup(s);
		 	if (opt_verbose) printf("bank #%d voice #%d %s\n", opt_bank, wanted_patch, s);
			tonebank[opt_bank] = TRUE;
		 }
	 }

	 global_preset_velmin = preset_velmin = -1;
	 global_preset_velmax = preset_velmax = -1;
	 global_preset_keymin = preset_keymin = -1;
	 global_preset_keymax = preset_keymax = -1;

	 /* for each layer in this preset */
	 for (inum=0; inum<pindex_count; inum++) {
	    int global_instrument_layer, global_instrument_velmin, global_instrument_velmax,
	    	instrument_velmin, instrument_velmax;
	    int global_instrument_keymin, global_instrument_keymax,
	    	instrument_keymin, instrument_keymax;

	    pgen = &sf_preset_generators[pindex[inum].wGenNdx];
	    pgen_count = pindex[inum+1].wGenNdx - pindex[inum].wGenNdx;

	    if (pgen_count > 0 && pgen[pgen_count-1].sfGenOper != SFGEN_instrument)
		    global_preset_layer = TRUE;
	    else global_preset_layer = FALSE;

	    if (global_preset_velmin >= 0) preset_velmin = global_preset_velmin;
	    if (global_preset_velmax >= 0) preset_velmax = global_preset_velmax;
	    if (global_preset_keymin >= 0) preset_keymin = global_preset_keymin;
	    if (global_preset_keymax >= 0) preset_keymax = global_preset_keymax;

	    if (pgen_count < 0) break;

	    if (pgen[0].sfGenOper == SFGEN_keyRange) {
		preset_keymin = pgen[0].genAmount.ranges.byLo;
		preset_keymax = pgen[0].genAmount.ranges.byHi;
		if (global_preset_layer) {
			global_preset_keymin = preset_keymin;
			global_preset_keymax = preset_keymax;
		}
	    }

	    for (jnum=0; jnum<pgen_count; jnum++) {
	     if (pgen[jnum].sfGenOper == SFGEN_velRange) {
		preset_velmin = pgen[jnum].genAmount.ranges.byLo;
		preset_velmax = pgen[jnum].genAmount.ranges.byHi;
		if (global_preset_layer) {
			global_preset_velmin = preset_velmin;
			global_preset_velmax = preset_velmax;
		}
	     }
	    }

	    /* find what instrument we should use */
	    if ((pgen_count > 0) && 
		(pgen[pgen_count-1].sfGenOper == SFGEN_instrument)) {

	       iheader = &sf_instruments[pgen[pgen_count-1].genAmount.wAmount];

	       iindex = &sf_instrument_indexes[iheader->wInstBagNdx];
	       iindex_count = iheader[1].wInstBagNdx - iheader[0].wInstBagNdx;

	       global_instrument_velmin = instrument_velmin = -1;
	       global_instrument_velmax = instrument_velmax = -1;
	       global_instrument_keymin = instrument_keymin = -1;
	       global_instrument_keymax = instrument_keymax = -1;


	       /* for each layer in this instrument */
	       for (lnum=0; lnum<iindex_count; lnum++) {
		  igen = &sf_instrument_generators[iindex[lnum].wInstGenNdx];
		  igen_count = iindex[lnum+1].wInstGenNdx - iindex[lnum].wInstGenNdx;

		  if (global_instrument_velmin >= 0) instrument_velmin = global_instrument_velmin;
		  if (global_instrument_velmax >= 0) instrument_velmax = global_instrument_velmax;
		  if (global_instrument_keymin >= 0) instrument_keymin = global_instrument_keymin;
		  if (global_instrument_keymax >= 0) instrument_keymax = global_instrument_keymax;

		  if ((igen_count > 0) && 
		      (igen[igen_count-1].sfGenOper != SFGEN_sampleID))
			  global_instrument_layer = TRUE;
		  else global_instrument_layer = FALSE;


		  for (jnum=0; jnum<igen_count; jnum++) {
		     if (igen[jnum].sfGenOper == SFGEN_velRange) {
			instrument_velmax = igen[jnum].genAmount.ranges.byHi;
			instrument_velmin = igen[jnum].genAmount.ranges.byLo;
			if (global_instrument_layer) {
				global_instrument_velmin = instrument_velmin;
				global_instrument_velmax = instrument_velmax;
			}
		     }
		  }

		  if (igen_count > 0 && igen[0].sfGenOper == SFGEN_keyRange) {
			instrument_keymax = igen[0].genAmount.ranges.byHi;
			instrument_keymin = igen[0].genAmount.ranges.byLo;
			if (global_instrument_layer) {
				global_instrument_keymin = instrument_keymin;
				global_instrument_keymax = instrument_keymax;
			}
		  }


		  if (instrument_velmin >= 0) velmin = instrument_velmin;
		  else velmin = 0;
		  if (instrument_velmax >= 0) velmax = instrument_velmax;
		  else velmax = 127;
		  if (preset_velmin >= 0 && preset_velmax >= 0) {
			  if (preset_velmin >= velmin && preset_velmax <= velmax) {
				  velmin = preset_velmin;
				  velmax = preset_velmax;
			  }
		  }

		  if (instrument_keymin >= 0) keymin = instrument_keymin;
		  else keymin = 0;
		  if (instrument_keymax >= 0) keymax = instrument_keymax;
		  else keymax = 127;
		  if (preset_keymin >= 0 && preset_keymax >= 0) {
			  if (preset_keymin >= keymin && preset_keymax <= keymax) {
				  keymin = preset_keymin;
				  keymax = preset_keymax;
			  }
		  }

		  drumnum = keymin;

		  /* find what sample we should use */
		  if ((igen_count > 0) && 
		      (igen[igen_count-1].sfGenOper == SFGEN_sampleID)) {
		     int i;

		     sample = &sf_samples[igen[igen_count-1].genAmount.wAmount];

		     /* sample->wSampleLink is the link?? */
		     /* lsample = &sf_samples[sample->wSampleLink] */
		     if (sample->sfSampleType & LINKED_SAMPLE) {
			     printf("linked sample: link is %d\n", sample->wSampleLink);
		     }

		     if (sample->sfSampleType & LINKED_SAMPLE) continue; /* linked */

		     s = sample->achSampleName;
		     i = strlen(s)-1;

		     if (s[i] == 'L') sample->sfSampleType = LEFT_SAMPLE;
		     if (s[i] == 'R') sample->sfSampleType = RIGHT_SAMPLE;

		     /* prettify the sample name */
		     s = getname(sample->achSampleName);

		     if (sample->sfSampleType & 0x8000) {
			printf("This SoundFont uses AWE32 ROM data in sample %s\n", s);
			if (opt_veryverbose)
			   printf("\n");
			continue;
		     }

	 	     if (drum) {
			 int pool_num;
			 for (pool_num = keymin; pool_num <= keymax; pool_num++) {
			     drumnum = pool_num;
		 	     if (!drum_name[opt_drum_bank][drumnum]) {
		 		     drum_name[opt_drum_bank][drumnum] = strdup(s);
		 		     if (opt_verbose) printf("drumset #%d drum #%d %s\n", opt_drum_bank, drumnum, s);
		 	     }
			     if (sample->sfSampleType == LEFT_SAMPLE)
		 	     	drum_samples_left[opt_drum_bank][drumnum]++;
			     else if (sample->sfSampleType == RIGHT_SAMPLE)
		 	     	drum_samples_right[opt_drum_bank][drumnum]++;
			     else drum_samples_mono[opt_drum_bank][drumnum]++;
			     record_velocity_range(drum, opt_drum_bank, drumnum,
					     velmin, velmax, sample->sfSampleType);
			 }
	 	     }
		     else {
			     if (sample->sfSampleType == LEFT_SAMPLE)
		 		voice_samples_left[opt_bank][wanted_patch]++;
			     else if (sample->sfSampleType == RIGHT_SAMPLE)
		 		voice_samples_right[opt_bank][wanted_patch]++;
			     else voice_samples_mono[opt_bank][wanted_patch]++;
			     record_velocity_range(0, opt_bank, wanted_patch,
					     velmin, velmax, sample->sfSampleType);
		     }
		  }

	       }
	    }
	 }


   }

   return TRUE;
}

static void shorten_drum_names(void)
{
	int i, j, velcount, right_patches;
	VelocityRangeList *vlist;

	for (i = 0; i < 128; i++) {
		if (drumset_name[i]) {
			for (j = 0; j < 128; j++) {
				if (drum_name[i][j]) {
	    				vlist = drum_velocity[i][j];
	    				if (vlist) {
						velcount = vlist->range_count;
						right_patches = vlist->right_patches[0];
					}
	    				else {
						continue;
					}
					if (right_patches) {
						char *dnm = drum_name[i][j];
						int name_len = strlen(dnm);
						if (name_len > 4 && dnm[name_len-1] == 'L' &&
								dnm[name_len-2] == '-')
							dnm[name_len-2] = '\0';
					}
				}
			}
		}
	}
}


static void gen_config_file(void)
{
	int i, j, velcount, right_patches;
	VelocityRangeList *vlist;

	if (opt_no_write) return;

	printf("Generating config file.\n");

	for (i = 0; i < 128; i++) {
		if (tonebank[i]) {
			fprintf(cfg_fd, "\nbank %d #N %s\n", i, tonebank_name[i]);
			for (j = 0; j < 128; j++) {
				if (voice_name[i][j]) {
	    				vlist = voice_velocity[i][j];
	    				if (vlist) {
						velcount = vlist->range_count;
						right_patches = vlist->right_patches[0];
					}
	    				else {
						fprintf(cfg_fd, "\t# %d %s could not be extracted\n", j,
					           voice_name[i][j]);
						continue;
					}
					fprintf(cfg_fd, "\t%d %s/%s", j,
					       tonebank_name[i], voice_name[i][j]);
					if (velcount > 1) fprintf(cfg_fd, "\t# %d velocity ranges", velcount);
					if (right_patches) {
						if (velcount == 1) fprintf(cfg_fd, "\t# stereo");
						else fprintf(cfg_fd,", stereo");
					}
					fprintf(cfg_fd, "\n");
				}
			}
		}
	}
	for (i = 0; i < 128; i++) {
		if (drumset_name[i]) {
			fprintf(cfg_fd, "\ndrumset %d #N %s\n", i, drumset_short_name[i]);
			for (j = 0; j < 128; j++) {
				if (drum_name[i][j]) {
	    				vlist = drum_velocity[i][j];
	    				if (vlist) {
						velcount = vlist->range_count;
						right_patches = vlist->right_patches[0];
					}
	    				else {
						fprintf(cfg_fd, "\t# %d %s could not be extracted\n", j,
					           drum_name[i][j]);
						continue;
					}
					fprintf(cfg_fd, "\t%d %s/%s", j,
					       drumset_name[i], drum_name[i][j]);
					if (velcount > 1) fprintf(cfg_fd, "\t# %d velocity ranges", velcount);
					if (right_patches) {
						if (velcount == 1) fprintf(cfg_fd, "\t# stereo");
						else fprintf(cfg_fd,", stereo");
					}
					fprintf(cfg_fd, "\n");
				}
			}
		}
	}
}


static void sort_velocity_layers(void)
{
    int i, j, k, velmin, velmax, velcount, left_patches, right_patches, mono_patches;
    int width, widest;
    VelocityRangeList *vlist;


    for (i = 0; i < 128; i++) {
    	if (tonebank[i]) {
	    for (j = 0; j < 128; j++) {
	    	if (voice_name[i][j]) {
        		vlist = voice_velocity[i][j];
        		if (vlist) {
	    			velcount = vlist->range_count;
				widest = 0;
				width = vlist->velmax[0] - vlist->velmin[0];
				for (k = 1; k < velcount; k++) {
					if (vlist->velmax[k] - vlist->velmin[k] > width) {
						widest = k;
						width = vlist->velmax[k] - vlist->velmin[k];
					}
				}
				if (widest) {
					velmin = vlist->velmin[0];
					velmax = vlist->velmax[0];
					mono_patches = vlist->mono_patches[0];
					left_patches = vlist->left_patches[0];
					right_patches = vlist->right_patches[0];

					vlist->velmin[0] = vlist->velmin[widest];
					vlist->velmax[0] = vlist->velmax[widest];
					vlist->mono_patches[0] = vlist->mono_patches[widest];
					vlist->left_patches[0] = vlist->left_patches[widest];
					vlist->right_patches[0] = vlist->right_patches[widest];

					vlist->velmin[widest] = velmin;
					vlist->velmax[widest] = velmax;
					vlist->mono_patches[widest] = mono_patches;
					vlist->left_patches[widest] = left_patches;
					vlist->right_patches[widest] = right_patches;
				}
	    		}
        		else continue;
	    	}
	    }
    	}
    }
    for (i = 0; i < 128; i++) {
    	if (drumset_name[i]) {
	    for (j = 0; j < 128; j++) {
	    	if (drum_name[i][j]) {
        		vlist = drum_velocity[i][j];
        		if (vlist) {
	    			velcount = vlist->range_count;
				widest = 0;
				width = vlist->velmax[0] - vlist->velmin[0];
				for (k = 1; k < velcount; k++) {
					if (vlist->velmax[k] - vlist->velmin[k] > width) {
						widest = k;
						width = vlist->velmax[k] - vlist->velmin[k];
					}
				}
				if (widest) {
					velmin = vlist->velmin[0];
					velmax = vlist->velmax[0];
					mono_patches = vlist->mono_patches[0];
					left_patches = vlist->left_patches[0];
					right_patches = vlist->right_patches[0];

					vlist->velmin[0] = vlist->velmin[widest];
					vlist->velmax[0] = vlist->velmax[widest];
					vlist->mono_patches[0] = vlist->mono_patches[widest];
					vlist->left_patches[0] = vlist->left_patches[widest];
					vlist->right_patches[0] = vlist->right_patches[widest];

					vlist->velmin[widest] = velmin;
					vlist->velmax[widest] = velmax;
					vlist->mono_patches[widest] = mono_patches;
					vlist->left_patches[widest] = left_patches;
					vlist->right_patches[widest] = right_patches;
				}
	    		}
        		else continue;
	    	}
	    }
    	}
    }
}

static void make_directories(void)
{
	int i, rcode, tonebank_count = 0;
	char tmpname[80];

	printf("Making bank directories.\n");

	for (i = 0; i < 128; i++) if (tonebank[i]) tonebank_count++;

	for (i = 0; i < 128; i++) {
		if (tonebank[i]) {
			if (tonebank_count > 1) {
				sprintf(tmpname, "%s-B%d", basename, i);
				tonebank_name[i] = strdup(tmpname);
			}
			else tonebank_name[i] = strdup(basename);
			if (opt_no_write) continue;
			if ( (rcode = access(tonebank_name[i], R_OK|W_OK|X_OK)) )
				rcode=mkdir(tonebank_name[i], S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
			if (rcode) {
				fprintf(stderr, "Could not create directory %s\n", tonebank_name[i]);
				exit(1);
			}
		}
	}
	if (opt_no_write) return;
	for (i = 0; i < 128; i++) {
		if (drumset_name[i]) {
			if ( (rcode = access(drumset_name[i], R_OK|W_OK|X_OK)) )
				rcode=mkdir(drumset_name[i], S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
			if (rcode) {
				fprintf(stderr, "Could not create directory %s\n", drumset_name[i]);
				exit(1);
			}
		}
	}
}

static void make_patch_files(void)
{
    int i, j, k, rcode, velcount, right_patches;
    char tmpname[80];
    FILE *pf;
    VelocityRangeList *vlist;
    int abort_this_one;

    printf("Melodic patch files.\n");
    for (i = 0; i < 128; i++) {
	abort_this_one = FALSE;
	if (tonebank[i]) for (j = 0; j < 128; j++) if (voice_name[i][j]) {
	    abort_this_one = FALSE;
	    vlist = voice_velocity[i][j];
	    if (vlist) velcount = vlist->range_count;
	    else velcount = 1;
	    if (opt_small) velcount = 1;
	    opt_bank = i;
	    opt_header = TRUE;
	    for (k = 0; k < velcount; k++) {
		if (vlist) {
			wanted_velmin = vlist->velmin[k];
			wanted_velmax = vlist->velmax[k];
			right_patches = vlist->right_patches[k];
		}
		else {
			wanted_velmin = 0;
			wanted_velmax = 127;
			right_patches = voice_samples_right[i][j];
		}
		opt_left_channel = TRUE;
		opt_right_channel = FALSE;
    		if (!grab_soundfont(j, FALSE, voice_name[i][j])) {
			fprintf(stderr, "Could not create patch %s for bank %s\n",
				       	voice_name[i][j], tonebank_name[i]);
			fprintf(stderr,"\tlayer %d of %d layer(s)\n", k+1, velcount);
			if (voice_velocity[i][j]) free(voice_velocity[i][j]);
			voice_velocity[i][j] = NULL;
			abort_this_one = TRUE;
			break;
		}
		opt_header = FALSE;
		if (abort_this_one) continue;
		if (vlist) right_patches = vlist->right_patches[k];
		if (right_patches && !opt_mono) {
			opt_left_channel = FALSE;
			opt_right_channel = TRUE;
    			if (!grab_soundfont(j, FALSE, voice_name[i][j])) {
				fprintf(stderr, "Could not create right patch %s for bank %s\n",
					       	voice_name[i][j], tonebank_name[i]);
				fprintf(stderr,"\tlayer %d of %d layer(s)\n", k+1, velcount);
				if (voice_velocity[i][j]) free(voice_velocity[i][j]);
				voice_velocity[i][j] = NULL;
				abort_this_one = TRUE;
				break;
			}
		}
	    }
	    if (abort_this_one || opt_no_write) continue;
	    sprintf(tmpname, "%s/%s.pat", tonebank_name[i], voice_name[i][j]);
	    if (!(pf = fopen(tmpname, "wb"))) {
			fprintf(stderr, "\nCould not open patch file %s\n", tmpname);
			if (voice_velocity[i][j]) free(voice_velocity[i][j]);
			voice_velocity[i][j] = NULL;
			continue;
	    }
	    if ( fwrite(mem, 1, mem_size, pf) != mem_size ) {
			fprintf(stderr, "\nCould not write to patch file %s\n", tmpname);
			if (voice_velocity[i][j]) free(voice_velocity[i][j]);
			voice_velocity[i][j] = NULL;
	    }
	    fclose(pf);
	}
    }
    printf("\nDrum patch files.\n");
    for (i = 0; i < 128; i++) {
	abort_this_one = FALSE;
	if (drumset_name[i]) for (j = 0; j < 128; j++) if (drum_name[i][j]) {
	    abort_this_one = FALSE;
	    vlist = drum_velocity[i][j];
	    if (vlist) velcount = vlist->range_count;
	    else velcount = 1;
	    if (!vlist) fprintf(stderr, "Uh oh, drum #%d %s has no velocity list\n", i, drumset_name[i]);
	    if (opt_small) velcount = 1;
	    opt_drum_bank = i;
	    opt_header = TRUE;
	    for (k = 0; k < velcount; k++) {
		if (vlist) {
			wanted_velmin = vlist->velmin[k];
			wanted_velmax = vlist->velmax[k];
			right_patches = vlist->right_patches[k];
		}
		else {
			wanted_velmin = 0;
			wanted_velmax = 127;
			right_patches = drum_samples_right[i][j];
		}
		opt_left_channel = TRUE;
		opt_right_channel = FALSE;
    		if (!grab_soundfont(j, TRUE, drum_name[i][j])) {
			fprintf(stderr, "Could not create left/mono patch %s for bank %s\n",
				       	drum_name[i][j], drumset_name[i]);
			fprintf(stderr,"\tlayer %d of %d layer(s)\n", k+1, velcount);
			if (drum_velocity[i][j]) free(drum_velocity[i][j]);
	    		drum_velocity[i][j] = NULL;
			abort_this_one = TRUE;
			break;
		}
		opt_header = FALSE;
		if (abort_this_one) continue;
		if (vlist) right_patches = vlist->right_patches[k];
		if (right_patches && !opt_mono) {
			opt_left_channel = FALSE;
			opt_right_channel = TRUE;
    			if (!grab_soundfont(j, TRUE, drum_name[i][j])) {
				fprintf(stderr, "Could not create right patch %s for bank %s\n",
					       	drum_name[i][j], drumset_name[i]);
				fprintf(stderr,"\tlayer %d of %d layer(s)\n", k+1, velcount);
				if (drum_velocity[i][j]) free(drum_velocity[i][j]);
	    			drum_velocity[i][j] = NULL;
				abort_this_one = TRUE;
				break;
			}
		}
	    }
	    if (abort_this_one || opt_no_write) continue;
	    sprintf(tmpname, "%s/%s.pat", drumset_name[i], drum_name[i][j]);
	    if (!(pf = fopen(tmpname, "wb"))) {
			fprintf(stderr, "\nCould not open patch file %s\n", tmpname);
			if (drum_velocity[i][j]) free(drum_velocity[i][j]);
	    		drum_velocity[i][j] = NULL;
			continue;
	    }
	    if ( fwrite(mem, 1, mem_size, pf) != mem_size ) {
			fprintf(stderr, "\nCould not write to patch file %s\n", tmpname);
			if (drum_velocity[i][j]) free(drum_velocity[i][j]);
	    		drum_velocity[i][j] = NULL;
	    }
	    fclose(pf);
	}
    }
    printf("\n");
}

/* reads a byte from the input file */
static int get8(FILE *f)
{
   return getc(f);
}



/* reads a word from the input file (little endian) */
static int get16(FILE *f)
{
   int b1, b2;

   b1 = get8(f);
   b2 = get8(f);

 if (big_endian)
   return ((b1 << 8) | b2);
 else
   return ((b2 << 8) | b1);
}



/* reads a long from the input file (little endian) */
static int get32(FILE *f)
{
   int b1, b2, b3, b4;

   b1 = get8(f);
   b2 = get8(f);
   b3 = get8(f);
   b4 = get8(f);

 if (big_endian)
   return ((b1 << 24) | (b2 << 16) | (b3 << 8) | b4);
 else
   return ((b4 << 24) | (b3 << 16) | (b2 << 8) | b1);
}



/* calculates the file offset for the end of a chunk */
static void calc_end(RIFF_CHUNK *chunk, FILE *f)
{
   chunk->end = ftell(f) + chunk->size + (chunk->size & 1);
}


/* reads and displays a SoundFont text/copyright message */
static void print_sf_string(FILE *f, char *title)
{
   char buf[256];
   char ch;
   int i = 0;

   do {
      ch = get8(f);
      buf[i++] = ch;
   } while ((ch) && (i < 256));

   if (i & 1)
      get8(f);

   if (!strncmp(title, "Made", 4)) {
	   strcpy(cpyrt, "Made by ");
	   strcat(cpyrt, buf);
   }
   
   if (!opt_no_write) fprintf(cfg_fd, "# %-12s%s\n", title, buf);
}



/* error handling macro */
#define BAD_SF()                                            \
{                                                           \
   fprintf(stderr, "Error: bad SoundFont structure\n");     \
   err = 1;                                                 \
   goto getout;                                             \
}



/* creates all the required patch files */
static void add_soundfont_patches(void)
{
   RIFF_CHUNK file, chunk, subchunk;
   FILE *f;
   int i;

   if (opt_verbose)
      printf("\nReading %s\n\n", opt_soundfont);
   else
      printf("Reading %s\n", opt_soundfont);

   f = fopen(opt_soundfont, "rb");
   if (!f) {
      fprintf(stderr, "Error opening file\n");
      err = 1;
      return;
   }

   file.id = get32(f);
   if (file.id != CID_RIFF) {
      fprintf(stderr, "Error: bad SoundFont header\n");
      err = 1;
      goto getout;
   }

   file.size = get32(f);
   calc_end(&file, f);
   file.type = get32(f);
   if (file.type != CID_sfbk) {
      fprintf(stderr, "Error: bad SoundFont header\n");
      err = 1;
      goto getout;
   }

   while (ftell(f) < file.end) {
      chunk.id = get32(f);
      chunk.size = get32(f);
      calc_end(&chunk, f);

      switch (chunk.id) {

	 case CID_LIST: 
	    /* a list of other chunks */
	    chunk.type = get32(f);

	    while (ftell(f) < chunk.end) {
	       subchunk.id = get32(f);
	       subchunk.size = get32(f);
	       calc_end(&subchunk, f);

	       switch (chunk.type) {

		  case CID_INFO: 
		     /* information block */
		     switch (subchunk.id) {

			case CID_ifil:
			   if (get16(f) < 2) {
			      fprintf(stderr, "Error: this is a SoundFont 1.x file, and I only understand version 2 (.sf2)\n");
			      err = 1;
			      goto getout;
			   }
			   get16(f);
			   break;

			case CID_INAM:
			   print_sf_string(f, "Bank name:");
			   break;

			case CID_irom:
			   print_sf_string(f, "ROM name:");
			   break;

			case CID_ICRD: 
			   print_sf_string(f, "Date:");
			   break;

			case CID_IENG: 
			   print_sf_string(f, "Made by:");
			   break;

			case CID_IPRD: 
			   print_sf_string(f, "Target:");
			   break;

			case CID_ICOP: 
			   print_sf_string(f, "Copyright:");
			   break;

			case CID_ISFT: 
			   print_sf_string(f, "Tools:");
			   break;
		     }

		     /* skip unknown chunks and extra data */
		     fseek(f, subchunk.end, SEEK_SET);
		     break;

		  case CID_pdta: 
		     /* preset, instrument and sample header data */
		     switch (subchunk.id) {

			case CID_phdr: 
			   /* preset headers */
			   sf_num_presets = subchunk.size/38;

			   if ((sf_num_presets*38 != subchunk.size) ||
			       (sf_num_presets < 2) || (sf_presets))
			      BAD_SF();

			   sf_presets = malloc(sizeof(sfPresetHeader) * sf_num_presets);

			   for (i=0; i<sf_num_presets; i++) {
			      fread(sf_presets[i].achPresetName, 20, 1, f);
			      sf_presets[i].wPreset = get16(f);
			      sf_presets[i].wBank = get16(f);
			      sf_presets[i].wPresetBagNdx = get16(f);
			      sf_presets[i].dwLibrary = get32(f);
			      sf_presets[i].dwGenre = get32(f);
			      sf_presets[i].dwMorphology = get32(f);
			   }
			   break;

			case CID_pbag: 
			   /* preset index list */
			   sf_num_preset_indexes = subchunk.size/4;

			   if ((sf_num_preset_indexes*4 != subchunk.size) || 
			       (sf_preset_indexes))
			      BAD_SF();

			   sf_preset_indexes = malloc(sizeof(sfPresetBag) * sf_num_preset_indexes);

			   for (i=0; i<sf_num_preset_indexes; i++) {
			      sf_preset_indexes[i].wGenNdx = get16(f);
			      sf_preset_indexes[i].wModNdx = get16(f);
			   }
			   break;

			case CID_pgen:
			   /* preset generator list */
			   sf_num_preset_generators = subchunk.size/4;

			   if ((sf_num_preset_generators*4 != subchunk.size) || 
			       (sf_preset_generators))
			      BAD_SF();

			   sf_preset_generators = malloc(sizeof(sfGenList) * sf_num_preset_generators);

			   for (i=0; i<sf_num_preset_generators; i++) {
			      sf_preset_generators[i].sfGenOper = get16(f);
			      sf_preset_generators[i].genAmount.wAmount = get16(f);
			   }
			   break;

			case CID_inst: 
			   /* instrument names and indices */
			   sf_num_instruments = subchunk.size/22;

			   if ((sf_num_instruments*22 != subchunk.size) ||
			       (sf_num_instruments < 2) || (sf_instruments))
			      BAD_SF();

			   sf_instruments = malloc(sizeof(sfInst) * sf_num_instruments);

			   for (i=0; i<sf_num_instruments; i++) {
			      fread(sf_instruments[i].achInstName, 20, 1, f);
			      sf_instruments[i].wInstBagNdx = get16(f);
			   }
			   break;

			case CID_ibag: 
			   /* instrument index list */
			   sf_num_instrument_indexes = subchunk.size/4;

			   if ((sf_num_instrument_indexes*4 != subchunk.size) ||
			       (sf_instrument_indexes))
			      BAD_SF();

			   sf_instrument_indexes = malloc(sizeof(sfInstBag) * sf_num_instrument_indexes);

			   for (i=0; i<sf_num_instrument_indexes; i++) {
			      sf_instrument_indexes[i].wInstGenNdx = get16(f);
			      sf_instrument_indexes[i].wInstModNdx = get16(f);
			   }
			   break;

			case CID_igen: 
			   /* instrument generator list */
			   sf_num_instrument_generators = subchunk.size/4;

			   if ((sf_num_instrument_generators*4 != subchunk.size) ||
			       (sf_instrument_generators))
			      BAD_SF();

			   sf_instrument_generators = malloc(sizeof(sfGenList) * sf_num_instrument_generators);

			   for (i=0; i<sf_num_instrument_generators; i++) {
			      sf_instrument_generators[i].sfGenOper = get16(f);
			      sf_instrument_generators[i].genAmount.wAmount = get16(f);
			   }
			   break;

			case CID_shdr:
			   /* sample headers */
			   sf_num_samples = subchunk.size/46;

			   if ((sf_num_samples*46 != subchunk.size) ||
			       (sf_num_samples < 2) || (sf_samples))
			      BAD_SF();

			   sf_samples = malloc(sizeof(sfSample) * sf_num_samples);

			   for (i=0; i<sf_num_samples; i++) {
			      fread(sf_samples[i].achSampleName, 20, 1, f);
			      sf_samples[i].dwStart = get32(f);
			      sf_samples[i].dwEnd = get32(f);
			      sf_samples[i].dwStartloop = get32(f);
			      sf_samples[i].dwEndloop = get32(f);
			      sf_samples[i].dwSampleRate = get32(f);
			      sf_samples[i].byOriginalKey = get8(f);
			      sf_samples[i].chCorrection = get8(f);
			      sf_samples[i].wSampleLink = get16(f);
			      sf_samples[i].sfSampleType = get16(f);
			   }
			   break;
		     }

		     /* skip unknown chunks and extra data */
		     fseek(f, subchunk.end, SEEK_SET);
		     break;

		  case CID_sdta: 
		     /* sample data block */
		     switch (subchunk.id) {

			case CID_smpl: 
			   /* sample waveform (all in one) */
			   if (sf_sample_data)
			      BAD_SF();

			   sf_sample_data_size = subchunk.size / 2;
			   sf_sample_data = malloc(sizeof(short) * sf_sample_data_size);

			   for (i=0; i<sf_sample_data_size; i++)
			      sf_sample_data[i] = get16(f);

			   break;
		     }

		     /* skip unknown chunks and extra data */
		     fseek(f, subchunk.end, SEEK_SET);
		     break;

		  default: 
		     /* unrecognised chunk */
		     fseek(f, chunk.end, SEEK_SET);
		     break;
	       }
	    }
	    break;

	 default:
	    /* not a list so we're not interested */
	    fseek(f, chunk.end, SEEK_SET);
	    break;
      }

      if (feof(f))
	 BAD_SF();
   }

   getout:

   if (f)
      fclose(f);

   /* convert SoundFont to .pat format, and add it to the output datafile */
   if (!err) {
      if ((!sf_sample_data) || (!sf_presets) || 
	  (!sf_preset_indexes) || (!sf_preset_generators) || 
	  (!sf_instruments) || (!sf_instrument_indexes) || 
	  (!sf_instrument_generators) || (!sf_samples))
	 BAD_SF();

      if (opt_verbose)
	 printf("\n");

      grab_soundfont_banks();
      make_directories();
      sort_velocity_layers();
      shorten_drum_names();
      make_patch_files();
      gen_config_file();
   }


   /* oh, how polite I am... */
   if (sf_sample_data) {
      free(sf_sample_data);
      sf_sample_data = NULL;
   }

   if (sf_presets) {
      free(sf_presets);
      sf_presets = NULL;
   }

   if (sf_preset_indexes) {
      free(sf_preset_indexes);
      sf_preset_indexes = NULL;
   }

   if (sf_preset_generators) {
      free(sf_preset_generators);
      sf_preset_generators = NULL;
   }

   if (sf_instruments) {
      free(sf_instruments);
      sf_instruments = NULL;
   }

   if (sf_instrument_indexes) {
      free(sf_instrument_indexes);
      sf_instrument_indexes = NULL;
   }

   if (sf_instrument_generators) {
      free(sf_instrument_generators);
      sf_instrument_generators = NULL;
   }

   if (sf_samples) {
      free(sf_samples);
      sf_samples = NULL;
   }
}

extern char *optarg;
extern int optind, opterr, optopt;



int main(int argc, char *argv[])
{
	int i, c;
	char cfgname[80];
	char *inname;

	while ((c = getopt (argc, argv, "FVvnsdm")) > 0)
		switch (c) {
			case 'v':
	    			opt_verbose = 1;
	    			break;
			case 'n':
	    			opt_no_write = 1;
	    			break;
			case 's':
	    			opt_small = 1;
	    			break;
			case 'd':
	    			opt_drum = 1;
	    			break;
			case 'm':
	    			opt_mono = 1;
	    			break;
			case 'F':
	    			opt_adjust_sample_flags = 1;
	    			break;
			case 'V':
	    			opt_adjust_volume = 0;
	    			break;
			default:
				fprintf(stderr, "usage: unsf [-v] [-n] [-s] [-d] [-m] [-F] [-V] filename\n");
				return 1;
		}

	if (argc - optind != 1) {
		fprintf(stderr, "usage: unsf [-v] [-n] [-s] [-d] [-m] [-F] [-V] filename\n");
		exit(1);
	}


	inname = strrchr (argv [optind], '/');
	inname = inname ? inname + 1 : argv [optind];

	strcpy(basename, inname);
	inname = strrchr (basename, '.');
	if (inname) inname[0] = '\0';

	for (i = 0; i < strlen(basename); i++) {
		if (basename[i] == ' ') basename[i] = '_';
		else if (basename[i] == '#') basename[i] = '_';
	}

	strcpy(cfgname, basename);
	strcat(cfgname, ".cfg");
	if (!opt_no_write) {
		if ( !(cfg_fd = fopen(cfgname, "wb")) ) return 1;
	}

	opt_soundfont = argv[optind];

	byteorder();

	add_soundfont_patches();

	if (!opt_no_write) fclose(cfg_fd);

	return 0;
}
