#include "stdafx.h"
#include "CT_Blender.h"

// from buildinfo.c
#ifdef BUILD_DATE
extern char * build_date;
extern char * build_time;
extern char * build_platform;
extern char * build_type;
#endif

/*	Local Function prototypes */
static void print_help();
static void print_version();


/* defined in ghostwinlay and winlay, we can't include carbon here, conflict with DNA */
#ifdef __APPLE__
extern int checkAppleVideoCard(void);
extern void getMacAvailableBounds(short *top, short *left, short *bottom, short *right);
extern void	winlay_get_screensize(int *width_r, int *height_r);
extern void	winlay_process_events(int wait_for_event);
#endif


/* for the callbacks: */
extern "C" {
extern int pluginapi_force_ref(void);  /* from blenpluginapi:pluginapi.c */
}

char bprogname[FILE_MAXDIR+FILE_MAXFILE]; /* from blenpluginapi:pluginapi.c */
char btempdir[FILE_MAXDIR+FILE_MAXFILE];

/* Initialise callbacks for the modules that need them */
void setCallbacks(void); 

#if defined(__sgi) || defined(__alpha__)
static void fpe_handler(int sig)
{
	// printf("SIGFPE trapped\n");
}
#endif

/* handling ctrl-c event in console */
static void blender_esc(int sig)
{
	static int count = 0;

	G.afbreek = 1;	/* forces render loop to read queue, not sure if its needed */

	if (sig == 2) {
		if (count) {
			printf("\nBlender killed\n");
			exit(2);
		}
		printf("\nSent an internal break event. Press ^C again to kill Blender\n");
		count++;
	}
}

static void print_version(void)
{
#ifdef BUILD_DATE
	printf ("Blender %d.%02d (sub %d) Build\n", G.version/100, G.version%100, BLENDER_SUBVERSION);
	printf ("\tbuild date: %s\n", build_date);
	printf ("\tbuild time: %s\n", build_time);
	printf ("\tbuild revision: %s\n", build_rev);
	printf ("\tbuild platform: %s\n", build_platform);
	printf ("\tbuild type: %s\n", build_type);
#else
	printf ("Blender %d.%02d (sub %d) Build\n", G.version/100, G.version%100, BLENDER_SUBVERSION);
#endif
}

static void print_help(void)
{
	printf ("Blender %d.%02d (sub %d) Build\n", G.version/100, G.version%100, BLENDER_SUBVERSION);
	printf ("Usage: blender [args ...] [file] [args ...]\n");
	printf ("\nRender options:\n");
	printf ("  -b <file>\tRender <file> in background (doesn't load the user defaults .B.blend file)\n");
	printf ("    -a render frames from start to end (inclusive), only works when used after -b\n");
	printf ("    -S <name>\tSet scene <name>\n");
	printf ("    -f <frame>\tRender frame <frame> and save it\n");				
	printf ("    -s <frame>\tSet start to frame <frame> (use before the -a argument)\n");
	printf ("    -e <frame>\tSet end to frame <frame> (use before the -a argument)\n");
	printf ("    -o <path>\tSet the render path and file name.\n");
	printf ("      Use // at the start of the path to\n");
	printf ("        render relative to the blend file.\n");
	printf ("      The # characters are replaced by the frame number, and used to define zero padding.\n");
	printf ("        ani_##_test.png becomes ani_01_test.png\n");
	printf ("        test-######.png becomes test-000001.png\n");
	printf ("        When the filename has no #, The suffix #### is added to the filename\n");
	printf ("      The frame number will be added at the end of the filename.\n");
	printf ("      eg: blender -b foobar.blend -o //render_ -F PNG -x 1 -a\n");
	printf ("\nFormat options:\n");
	printf ("    -F <format>\tSet the render format, Valid options are...\n");
	printf ("    \tTGA IRIS HAMX JPEG MOVIE IRIZ RAWTGA\n");
	printf ("    \tAVIRAW AVIJPEG PNG BMP FRAMESERVER\n");
	printf ("    (formats that can be compiled into blender, not available on all systems)\n");
	printf ("    \tHDR TIFF EXR MULTILAYER MPEG AVICODEC QUICKTIME CINEON DPX DDS\n");
	printf ("    -x <bool>\tSet option to add the file extension to the end of the file.\n");
	printf ("    -t <threads>\tUse amount of <threads> for rendering (background mode only).\n");
	printf ("      [1-8], 0 for systems processor count.\n");
	printf ("\nAnimation playback options:\n");
	printf ("  -a <file(s)>\tPlayback <file(s)>, only operates this way when -b is not used.\n");
	printf ("    -p <sx> <sy>\tOpen with lower left corner at <sx>, <sy>\n");
	printf ("    -m\t\tRead from disk (Don't buffer)\n");
	printf ("    -f <fps> <fps-base>\t\tSpecify FPS to start with\n");

	printf ("\nWindow options:\n");
	printf ("  -w\t\tForce opening with borders (default)\n");
	printf ("  -W\t\tForce opening without borders\n");
	printf ("  -p <sx> <sy> <w> <h>\tOpen with lower left corner at <sx>, <sy>\n");
	printf ("                      \tand width and height <w>, <h>\n");
	printf ("\nGame Engine specific options:\n");
	printf ("  -g fixedtime\t\tRun on 50 hertz without dropping frames\n");
	printf ("  -g vertexarrays\tUse Vertex Arrays for rendering (usually faster)\n");
	printf ("  -g noaudio\t\tNo audio in Game Engine\n");
	printf ("  -g nomipmap\t\tNo Texture Mipmapping\n");
	printf ("  -g linearmipmap\tLinear Texture Mipmapping instead of Nearest (default)\n");

	printf ("\nMisc options:\n");
	printf ("  -d\t\tTurn debugging on\n");
	printf ("  -noaudio\tDisable audio on systems that support audio\n");
	printf ("  -h\t\tPrint this help text\n");
	printf ("  -y\t\tDisable automatic python script execution (scriptlinks, pydrivers, pyconstraints, pynodes)\n");
	printf ("  -P <filename>\tRun the given Python script (filename or Blender Text)\n");
#ifdef WIN32
	printf ("  -R\t\tRegister .blend extension\n");
#endif
	printf ("  -v\t\tPrint Blender version and exit\n");
	printf ("  --\t\tEnds option processing.  Following arguments are \n");
	printf ("    \t\t   passed unchanged.  Access via Python's sys.argv\n");
	printf ("\nEnvironment Variables:\n");
	printf ("  $HOME\t\t\tStore files such as .blender/ .B.blend .Bfs .Blog here.\n");
#ifdef WIN32
	printf ("  $TEMP\t\tStore temporary files here.\n");
#else
	printf ("  $TMP or $TMPDIR\tStore temporary files here.\n");
	printf ("  $SDL_AUDIODRIVER\tLibSDL audio driver - alsa, esd, alsa, dma.\n");
	printf ("  $BF_TIFF_LIB\t\tUse an alternative libtiff.so for loading tiff image files.\n");
#endif
	printf ("  $IMAGEEDITOR\t\tImage editor executable, launch with the IKey from the file selector.\n");
	printf ("  $WINEDITOR\t\tText editor executable, launch with the EKey from the file selector.\n");
	printf ("  $PYTHONHOME\t\tPath to the python directory, eg. /usr/lib/python.\n");
	printf ("\nNote: Arguments must be separated by white space. eg:\n");
	printf ("    \"blender -ba test.blend\"\n");
	printf ("  ...will ignore the 'a'\n");
	printf ("    \"blender -b test.blend -f8\"\n");
	printf ("  ...will ignore 8 because there is no space between the -f and the frame value\n");
	printf ("Note: Arguments are executed in the order they are given. eg:\n");
	printf ("    \"blender -b test.blend -f 1 -o /tmp\"\n");
	printf ("  ...may not render to /tmp because '-f 1' renders before the output path is set\n");
	printf ("    \"blender -b -o /tmp test.blend -f 1\"\n");
	printf ("  ...may not render to /tmp because loading the blend file overwrites the output path that was set\n");
	printf ("    \"blender -b test.blend -o /tmp -f 1\" works as expected.\n\n");
}


double PIL_check_seconds_timer(void);
extern "C" {
extern void winlay_get_screensize(int *width_r, int *height_r);
}

static void main_init_screen( void )
{
	//setscreen(G.curscreen);

	if(G.main->scene.first==0) {
		//set_scene( add_scene("1") );
		G.scene = add_scene( "1" );
	}
}

int BL_Init(int argc, char **argv)
{
	int a, i, stax=0, stay=0, sizx, sizy, scr_init = 0;
	SYS_SystemHandle syshandle;

#if defined(WIN32) || defined (__linux__)
	int audio = 0;
#else
	int audio = 0;
#endif


#ifdef WITH_BINRELOC
	br_init( NULL );
#endif

	setCallbacks();
#ifdef __APPLE__
	/* patch to ignore argument finder gives us (pid?) */
	if (argc==2 && strncmp(argv[1], "-psn_", 5)==0) {
		extern int GHOST_HACK_getFirstFile(char buf[]);
		static char firstfilebuf[512];
		int scr_x,scr_y;

		argc= 1;

		/* first let us check if we are hardware accelerated and with VRAM > 16 Mo */

		if (checkAppleVideoCard()) {
			short top, left, bottom, right;

			winlay_get_screensize(&scr_x, &scr_y); 
			getMacAvailableBounds(&top, &left, &bottom, &right);
			setprefsize(left +10,scr_y - bottom +10,right-left -20,bottom - 64, 0);

		} else {
			winlay_get_screensize(&scr_x, &scr_y);

			/* 40 + 684 + (headers) 22 + 22 = 768, the powerbook screen height */
			setprefsize(120, 40, 850, 684, 0);
		}

		winlay_process_events(0);
		if (GHOST_HACK_getFirstFile(firstfilebuf)) {
			argc= 2;
			argv[1]= firstfilebuf;
		}
	}

#endif

#ifdef __FreeBSD__
	fpsetmask(0);
#endif
#ifdef __linux__
#ifdef __alpha__
	signal (SIGFPE, fpe_handler);
#else
	if ( getenv("SDL_AUDIODRIVER") == NULL) {
		setenv("SDL_AUDIODRIVER", "alsa", 1);
	}
#endif
#endif
#if defined(__sgi)
	signal (SIGFPE, fpe_handler);
#endif

	// copy path to executable in bprogname. playanim and creting runtimes
	// need this.

	BLI_where_am_i(bprogname, argv[0]);

	/* Hack - force inclusion of the plugin api functions,
	* see blenpluginapi:pluginapi.c
	*/
	pluginapi_force_ref();

	init_nodesystem();

	initglobals();	/* blender.c */

	syshandle = SYS_GetSystem();
	GEN_init_messaging_system();

	/* first test for background */

	G.f |= G_DOSCRIPTLINKS; /* script links enabled by default */

	for(a=1; a<argc; a++) {

		/* Handle unix and windows style help requests */
		if ((!strcmp(argv[a], "--help")) || (!strcmp(argv[a], "/?"))){
			print_help();
			exit(0);
		}

		/* end argument processing after -- */
		if (!strcmp( argv[a], "--")){
			a = argc;
			break;
		}

		/* Handle long version request */
		if (!strcmp(argv[a], "--version")){
			print_version();
			exit(0);
		}

		/* Handle -* switches */
		else if(argv[a][0] == '-') {
			switch(argv[a][1]) {
			case 'a': /* -b was not given, play an animation */
				playanim(argc-1, argv+1);
				exit(0);
				break;
			case 'b':
			case 'B':
				G.background = 1;
				a= argc;
				break;

			case 'y':
				G.f &= ~G_DOSCRIPTLINKS;
				break;

			case 'Y':
				printf ("-y was used to disable script links because,\n");
				printf ("\t-p being taken, Ton was of the opinion that Y\n");
				printf ("\tlooked like a split (disabled) snake, and also\n");
				printf ("\twas similar to a python's tongue (unproven).\n\n");

				printf ("\tZr agreed because it gave him a reason to add a\n");
				printf ("\tcompletely useless text into Blender.\n\n");

				printf ("\tADDENDUM! Ton, in defense, found this picture of\n");
				printf ("\tan Australian python, exhibiting her (his/its) forked\n");
				printf ("\tY tongue. It could be part of an H Zr retorted!\n\n");
				printf ("\thttp://www.users.bigpond.com/snake.man/\n");

				exit(252);

			case 'h':			
				print_help();
				exit(0);
			case 'v':
				print_version();
				exit(0);
			default:
				break;
			}
		}
	}
	

#ifdef __sgi
	setuid(getuid()); /* end superuser */
#endif

	/* for all platforms, even windos has it! */
	if(G.background) signal(SIGINT, blender_esc);	/* ctrl c out bg render */
	G.background = 1;

	/* background render uses this font too */
	BKE_font_register_builtin(datatoc_Bfont, datatoc_Bfont_size);

	init_def_material();

	winlay_get_screensize(&sizx, &sizy);
	stax=0;
	stay=0;

	if(G.background==0) {
		for(a=1; a<argc; a++) {
			if(argv[a][0] == '-') {
				switch(argv[a][1]) {
				case 'p':	/* prefsize */
					if (argc-a < 5) {
						printf ("-p requires four arguments\n");
						exit(1);
					}
					a++;
					stax= atoi(argv[a]);
					a++;
					stay= atoi(argv[a]);
					a++;
					sizx= atoi(argv[a]);
					a++;
					sizy= atoi(argv[a]);
					G.windowstate = G_WINDOWSTATE_BORDER;

					break;
				case 'd':
					G.f |= G_DEBUG;		/* std output printf's */ 
					printf ("Blender %d.%02d (sub %d) Build\n", G.version/100, G.version%100, BLENDER_SUBVERSION);
					MEM_set_memory_debug();
#ifdef NAN_BUILDINFO
					printf("Build: %s %s %s %s\n", build_date, build_time, build_platform, build_type);

#endif // NAN_BUILDINFO
					for (i = 0; i < argc; i++) {
						printf("argv[%d] = %s\n", i, argv[i]);
					}
					break;

				case 'w':
					G.windowstate = G_WINDOWSTATE_BORDER;
					break;
				case 'W':
					/* XXX, fixme mein, borderless on OSX */
					G.windowstate = G_WINDOWSTATE_FULLSCREEN;
					break;
				case 'R':
					/* Registering filetypes only makes sense on windows...      */
#ifdef WIN32
					RegisterBlendExtension(argv[0]);
#endif
					break;
				case 'n':
				case 'N':
					if (BLI_strcasecmp(argv[a], "-noaudio") == 0|| BLI_strcasecmp(argv[a], "-nosound") == 0) {
						/**
						notify the gameengine that no audio is wanted, even if the user didn't give
						the flag -g noaudio.
						*/

						SYS_WriteCommandLineInt(syshandle,"noaudio",1);
						audio = 0;
						if (G.f & G_DEBUG) printf("setting audio to: %d\n", audio);
					}
					break;
				}
			}
		}

		if ( (G.windowstate == G_WINDOWSTATE_BORDER) || (G.windowstate == G_WINDOWSTATE_FULLSCREEN)) 
			setprefsize(stax, stay, sizx, sizy, 0);

		BPY_start_python(argc, argv);

		/**
		* NOTE: sound_init_audio() *must be* after start_python,
		* at least on FreeBSD.
		* added note (ton): i removed it altogether
		*/

		//BIF_init(); /* loads .B.blend */


		BLI_where_is_temp( btempdir, 1 ); /* call after loading the .B.blend so we can read U.tempdir */

	}
	else {
		BPY_start_python(argc, argv);

		BIF_init();

		BLI_where_is_temp( btempdir, 0 ); /* call after loading the .B.blend so we can read U.tempdir */

		// (ton) Commented out. I have no idea whats thisfor... will mail around!
		// SYS_WriteCommandLineInt(syshandle,"noaudio",1);
		// audio = 0;
		// sound_init_audio();
		// if (G.f & G_DEBUG) printf("setting audio to: %d\n", audio);
	}

	/**
	* NOTE: the U.pythondir string is NULL until BIF_init() is executed,
	* so we provide the BPY_ function below to append the user defined
	* pythondir to Python's sys.path at this point.  Simply putting
	* BIF_init() before BPY_start_python() crashes Blender at startup.
	* Update: now this function also inits the bpymenus, which also depend
	* on U.pythondir.
	*/
	BPY_post_start_python();

#ifdef WITH_QUICKTIME

	quicktime_init();

#endif /* WITH_QUICKTIME */

	/* dynamically load libtiff, if available */
	libtiff_init();
	if (!G.have_libtiff && (G.f & G_DEBUG)) {
		printf("Unable to load: libtiff.\n");
		printf("Try setting the BF_TIFF_LIB environment variable if you want this support.\n");
		printf("Example: setenv BF_TIFF_LIB /usr/lib/libtiff.so\n");
	}

	/* OK we are ready for it */

	for(a=1; a<argc; a++) {
		if (G.afbreek==1) break;

		if(argv[a][0] == '-') {
			switch(argv[a][1]) {
			case '-':  /* -- ends argument processing */
				a = argc;
				break;

			case 'p':	/* prefsize */
				a+= 4;
				break;

			case 'g':
				{
					/**
					gameengine parameters are automaticly put into system
					-g [paramname = value]
					-g [boolparamname]
					example:
					-g novertexarrays
					-g maxvertexarraysize = 512
					*/

					if(++a < argc)
					{
						char* paramname = argv[a];
						/* check for single value versus assignment */
						if (a+1 < argc && (*(argv[a+1]) == '='))
						{
							a++;
							if (a+1 < argc)
							{
								a++;
								/* assignment */
								SYS_WriteCommandLineString(syshandle,paramname,argv[a]);
							}  else
							{
								printf("error: argument assignment (%s) without value.\n",paramname);
							}
							/* name arg eaten */

						} else
						{
							SYS_WriteCommandLineInt(syshandle,argv[a],1);

							/* doMipMap */
							if (!strcmp(argv[a],"nomipmap"))
							{
								set_mipmap(0); //doMipMap = 0;
							}
							/* linearMipMap */
							if (!strcmp(argv[a],"linearmipmap"))
							{
								set_linear_mipmap(1); //linearMipMap = 1;
							}


						} /* if (*(argv[a+1]) == '=') */
					} /*	if(++a < argc)  */
					break;
				}
			case 'f':
				a++;
				if (G.scene) {
					if (a < argc) {
						int frame= MIN2(MAXFRAME, MAX2(1, atoi(argv[a])));
						Render *re= RE_NewRender(G.scene->id.name);
						RE_BlenderAnim(re, G.scene, frame, frame);
					}
				} else {
					printf("\nError: no blend loaded. cannot use '-f'.\n");
				}
				break;
			case 'a':
				if (G.scene) {
					Render *re= RE_NewRender(G.scene->id.name);
					RE_BlenderAnim(re, G.scene, G.scene->r.sfra, G.scene->r.efra);
				} else {
					printf("\nError: no blend loaded. cannot use '-a'.\n");
				}
				break;
			case 'S':
				if(++a < argc) {
					set_scene_name(argv[a]);
				}
				break;
			case 's':
				a++;
				if(G.scene) {
					int frame= MIN2(MAXFRAME, MAX2(1, atoi(argv[a])));
					if (a < argc) (G.scene->r.sfra) = frame;
				} else {
					printf("\nError: no blend loaded. cannot use '-s'.\n");
				}
				break;
			case 'e':
				a++;
				if(G.scene) {
					int frame= MIN2(MAXFRAME, MAX2(1, atoi(argv[a])));
					if (a < argc) (G.scene->r.efra) = frame;
				} else {
					printf("\nError: no blend loaded. cannot use '-e'.\n");
				}
				break;
			case 'P':
				a++;
				if (a < argc) {
					/* If we're not running in background mode, then give python a valid screen */
					if ((G.background==0) && (scr_init==0)) {
						main_init_screen();
						scr_init = 1;
					}
					BPY_run_python_script (argv[a]);
				}
				else printf("\nError: you must specify a Python script after '-P '.\n");
				break;
			case 'o':
				a++;
				if (a < argc){
					if(G.scene) {
						BLI_strncpy(G.scene->r.pic, argv[a], FILE_MAXDIR);
					} else {
						printf("\nError: no blend loaded. cannot use '-o'.\n");
					}
				} else {
					printf("\nError: you must specify a path after '-o '.\n");
				}
				break;
			case 'F':
				a++;
				if (a < argc){
					if(!G.scene) {
						printf("\nError: no blend loaded. order the arguments so '-F ' is after the blend is loaded.\n");
					} else {
						if      (!strcmp(argv[a],"TGA")) G.scene->r.imtype = R_TARGA;
						else if (!strcmp(argv[a],"IRIS")) G.scene->r.imtype = R_IRIS;
						else if (!strcmp(argv[a],"HAMX")) G.scene->r.imtype = R_HAMX;
#ifdef WITH_DDS
						else if (!strcmp(argv[a],"DDS")) G.scene->r.imtype = R_DDS;
#endif
						else if (!strcmp(argv[a],"JPEG")) G.scene->r.imtype = R_JPEG90;
						else if (!strcmp(argv[a],"MOVIE")) G.scene->r.imtype = R_MOVIE;
						else if (!strcmp(argv[a],"IRIZ")) G.scene->r.imtype = R_IRIZ;
						else if (!strcmp(argv[a],"RAWTGA")) G.scene->r.imtype = R_RAWTGA;
						else if (!strcmp(argv[a],"AVIRAW")) G.scene->r.imtype = R_AVIRAW;
						else if (!strcmp(argv[a],"AVIJPEG")) G.scene->r.imtype = R_AVIJPEG;
						else if (!strcmp(argv[a],"PNG")) G.scene->r.imtype = R_PNG;
						else if (!strcmp(argv[a],"AVICODEC")) G.scene->r.imtype = R_AVICODEC;
						else if (!strcmp(argv[a],"QUICKTIME")) G.scene->r.imtype = R_QUICKTIME;
						else if (!strcmp(argv[a],"BMP")) G.scene->r.imtype = R_BMP;
						else if (!strcmp(argv[a],"HDR")) G.scene->r.imtype = R_RADHDR;
						else if (!strcmp(argv[a],"TIFF")) G.scene->r.imtype = R_TIFF;
#ifdef WITH_OPENEXR
						else if (!strcmp(argv[a],"EXR")) G.scene->r.imtype = R_OPENEXR;
						else if (!strcmp(argv[a],"MULTILAYER")) G.scene->r.imtype = R_MULTILAYER;
#endif
						else if (!strcmp(argv[a],"MPEG")) G.scene->r.imtype = R_FFMPEG;
						else if (!strcmp(argv[a],"FRAMESERVER")) G.scene->r.imtype = R_FRAMESERVER;
						else if (!strcmp(argv[a],"CINEON")) G.scene->r.imtype = R_CINEON;
						else if (!strcmp(argv[a],"DPX")) G.scene->r.imtype = R_DPX;
						else printf("\nError: Format from '-F' not known or not compiled in this release.\n");
					}
				} else {
					printf("\nError: no blend loaded. cannot use '-x'.\n");
				}
				break;

			case 't':
				a++;
				if(G.background) {
					RE_set_max_threads(atoi(argv[a]));
				} else {
					printf("Warning: threads can only be set in background mode\n");
				}
				break;
			case 'x': /* extension */
				a++;
				if (a < argc) {
					if(G.scene) {
						if (argv[a][0] == '0') {
							G.scene->r.scemode &= ~R_EXTENSION;
						} else if (argv[a][0] == '1') {
							G.scene->r.scemode |= R_EXTENSION;
						} else {
							printf("\nError: Use '-x 1' or '-x 0' To set the extension option.\n");
						}
					} else {
						printf("\nError: no blend loaded. order the arguments so '-o ' is after '-x '.\n");
					}
				} else {
					printf("\nError: you must specify a path after '- '.\n");
				}
				break;
			}
		}
		else {

			/* Make the path absolute because its needed for relative linked blends to be found */
			int abs = 0;
			int filelen;
			char cwd[FILE_MAXDIR + FILE_MAXFILE];
			char filename[FILE_MAXDIR + FILE_MAXFILE];
			cwd[0] = filename[0] = '\0';

			BLI_strncpy(filename, argv[a], sizeof(filename));
			filelen = strlen(filename);

			/* relative path checks, could do more tests here... */
#ifdef WIN32
			/* Account for X:/ and X:\ - should be enough */
			if (filelen >= 3 && filename[1] == ':' && (filename[2] == '\\' || filename[2] == '/'))
				abs = 1;
#else
			if (filelen >= 2 && filename[0] == '/')
				abs = 1	;
#endif
			if (!abs) {
				BLI_getwdN(cwd); /* incase the full path to the blend isnt used */

				if (cwd[0] == '\0') {
					printf(
						"Could not get the current working directory - $PWD for an unknown reason.\n\t"
						"Relative linked files will not load if the entire blend path is not used.\n\t"
						"The 'Play' button may also fail.\n"
						);
				} else {
					/* uses the blend path relative to cwd important for loading relative linked files.
					*
					* cwd should contain c:\ etc on win32 so the relbase can be NULL
					* relbase being NULL also prevents // being misunderstood as relative to the current
					* blend file which isnt a feature we want to use in this case since were dealing
					* with a path from the command line, rather then from inside Blender */

					BLI_make_file_string(NULL, filename, cwd, argv[a]); 
				}
			}

			if (G.background) {
				int retval = BKE_read_file(filename, NULL);
				sound_initialize_sounds();

				/*we successfully loaded a blend file, get sure that
				pointcache works */
				if (retval!=0) G.relbase_valid = 1;

				/* happens for the UI on file reading too */
				BKE_reset_undo();
				BKE_write_undo("original");	/* save current state */
			} else {
				/* we are not running in background mode here, but start blender in UI mode with 
				a file - this should do everything a 'load file' does */
				BIF_read_file(filename);
			}
		}
	}

	if(G.background) {
		/* actually incorrect, but works for now (ton) */
		//exit_usiblender();
	}

	if (scr_init==0) {
		main_init_screen();
	}

	//screenmain(); /* main display loop */

	return 0;
} /* end of int main(argc,argv)	*/

static void error_cb(char *err)
{
	MessageBoxA( 0, err, 0, 0 );
	//error("%s", err);
}

static void mem_error_cb(char *errorStr)
{
	fprintf(stderr, errorStr);
	fflush(stderr);
}

void setCallbacks(void)
{
	/* Error output from the alloc routines: */
	MEM_set_error_callback(mem_error_cb);


	/* BLI_blenlib: */

	BLI_setErrorCallBack(error_cb); /* */
	BLI_setInterruptCallBack(blender_test_break);

}

//////////////////////////////////////////////////////////////////////////
// 사용되는 블렌더 함수들
//////////////////////////////////////////////////////////////////////////

void BL_EnterEditmode( Object* pObj, int wc )
{
	Base *base;
	Object *ob;
	Mesh *me;
	bArmature *arm;
	int ok= 0;

	if(G.scene->id.lib) return;
	base= BASACT;
	if(base==0) return;
	if((G.vd==NULL || (base->lay & G.vd->lay))==0) return;

	strcpy(G.editModeTitleExtra, "");

	base->object = pObj;		
	ob = G.obedit = base->object;

	if(ob->data==0) return;

	if (object_data_is_libdata(ob)) {
		error_libdata();
		return;
	}

	if(ob->type==OB_MESH) {
		me= get_mesh(ob);
		if( me==0 ) return;
		if(me->pv) mesh_pmv_off(ob, me);
		ok= 1;
		G.obedit= ob;
		make_editMesh();
		//allqueue(REDRAWBUTSLOGIC, 0);
		/*if(G.f & G_FACESELECT) allqueue(REDRAWIMAGE, 0);*/
		//if (EM_texFaceCheck())
		//allqueue(REDRAWIMAGE, 0);

	}
	if (ob->type==OB_ARMATURE){
		arm= (bArmature*)base->object->data;
		if (!arm) return;
		/*
		* The function object_data_is_libdata make a problem here, the
		* check for ob->proxy return 0 and let blender enter to edit mode
		* this causa a crash when you try leave the edit mode.
		* The problem is that i can't remove the ob->proxy check from
		* object_data_is_libdata that prevent the bugfix #6614, so
		* i add this little hack here.
		*/
		if(arm->id.lib) {
			error_libdata();
			return;
		}
		ok=1;
		G.obedit=ob;
		make_editArmature();
		/* to ensure all goes in restposition and without striding */
		DAG_object_flush_update(G.scene, G.obedit, OB_RECALC);

		//allqueue (REDRAWVIEW3D,0);
	}
	else if(ob->type==OB_FONT) {
		G.obedit= ob;
		ok= 1;
		make_editText();
	}
	else if(ob->type==OB_MBALL) {
		G.obedit= ob;
		ok= 1;
		//make_editMball();
	}
	else if(ob->type==OB_LATTICE) {
		G.obedit= ob;
		ok= 1;
		make_editLatt();
	}
	else if(ob->type==OB_SURF || ob->type==OB_CURVE) {
		ok= 1;
		G.obedit= ob;
		make_editNurb();
	}
	//allqueue(REDRAWBUTSEDIT, 0);
	//allqueue(REDRAWOOPS, 0);
	countall();

	if(ok) {
		DAG_object_flush_update(G.scene, G.obedit, OB_RECALC_DATA);
	}
	else G.obedit= NULL;
}

void BL_ExitEditmode(int flag)	/* freedata==0 at render, 1= freedata, 2= do undo buffer too */
{
	Object *ob;
	int freedata = flag & EM_FREEDATA;

	if(G.obedit==NULL) return;

	if(G.obedit->type==OB_MESH) {

		/* temporal */
		countall();

		if(EM_texFaceCheck())
			allqueue(REDRAWIMAGE, 0);

		if(retopo_mesh_paint_check())
			retopo_end_okee();

		if(G.totvert>MESH_MAX_VERTS) {
			error("Too many vertices");
			return;
		}
		load_editMesh();

		if(freedata) free_editMesh(G.editMesh);

		if(G.f & G_WEIGHTPAINT)
			mesh_octree_table(G.obedit, NULL, 'e');
	}
	else if (G.obedit->type==OB_ARMATURE){	
		load_editArmature();
		if (freedata) free_editArmature();
	}
	else if ELEM(G.obedit->type, OB_CURVE, OB_SURF) {
		//extern ListBase editNurb;
		//load_editNurb();
		//if(freedata) freeNurblist(&editNurb);
	}
	else if(G.obedit->type==OB_FONT && freedata) {
		load_editText();
	}
	else if(G.obedit->type==OB_LATTICE) {
		load_editLatt();
		if(freedata) free_editLatt();
	}
	else if(G.obedit->type==OB_MBALL) {
		//extern ListBase editelems;
		//load_editMball();
		//if(freedata) BLI_freelistN(&editelems);
	}

	ob= G.obedit;

	/* for example; displist make is different in editmode */
	if(freedata) G.obedit= NULL;

	if(ob->type==OB_MESH && get_mesh(ob)->mr)
		multires_edge_level_update(ob, get_mesh(ob));

	/* also flush ob recalc, doesn't take much overhead, but used for particles */
	DAG_object_flush_update(G.scene, ob, OB_RECALC_OB|OB_RECALC_DATA);

	if(freedata) {
		setcursor_space(SPACE_VIEW3D, CURSOR_STD);
	}

	countall();
	//allqueue(REDRAWVIEW3D, 1);
	//allqueue(REDRAWBUTSALL, 0);
	//allqueue(REDRAWACTION, 0);
	//allqueue(REDRAWNLA, 0);
	//allqueue(REDRAWIPO, 0);
	//allqueue(REDRAWOOPS, 0);

	//scrarea_queue_headredraw(curarea);

	if(G.obedit==NULL && (flag & EM_FREEUNDO)) 
		BIF_undo_push("Editmode");

	//if(flag & EM_WAITCURSOR) waitcursor(0);
	G.obedit = NULL;
}

VOID
BL_RemoveVertexDoubles( Object* pObj, float limit )
{
	BL_EnterEditmode( pObj, 0 );
	EditMesh *em = G.editMesh;
	EditVert *eve;
	for(eve= (EditVert*)em->verts.first; eve; eve= eve->next) eve->f &= SELECT;
	removedoublesflag( 1, 0, limit );
	BL_ExitEditmode( 0 );	
	countall();
}

VOID
BL_ConvertToTriMesh( Object* pObj )
{
	BL_EnterEditmode( pObj, 0 );
	convert_to_triface( 0 );
	BL_ExitEditmode( 0 );
	countall();
}

VOID
BL_SubdivideFaces( Object* pObj )
{
	BL_EnterEditmode( pObj, 0 );
	EditMesh *em = G.editMesh;
	EditFace *ef;
	for(ef= (EditFace*)em->faces.first; ef; ef = ef->next) ef->f &= SELECT;
	esubdivideflag( 1, 0.0, 0, 1, 0 );	
	BL_ExitEditmode( 0 );
	countall();
}

FLOAT
BL_CalcDistanceVerts( MVert* v1, MVert* v2 )
{
	D3DXVECTOR3 vc1(v1->co[0], v1->co[1], v1->co[2]), vc2(v2->co[0], v2->co[1], v2->co[2]);
	return D3DXVec3Length( &(vc2 - vc1) );
}

static void 
build_edgecollection(ListBase *allcollections)
{
	EditEdge *eed;
	Collection *edgecollection, *newcollection;
	CollectedEdge *newedge;

	int currtag = 1;
	short ebalanced = 0;
	short collectionfound = 0;

	for (eed=(EditEdge*)G.editMesh->edges.first; eed; eed = (EditEdge*)eed->next){	
		eed->tmp.l = 0;
		eed->v1->tmp.l = 0;
		eed->v2->tmp.l = 0;
	}

	/*1st pass*/
	for(eed=(EditEdge*)G.editMesh->edges.first; eed; eed=(EditEdge*)eed->next){
		if(eed->f&SELECT){
			eed->v1->tmp.l = currtag;
			eed->v2->tmp.l = currtag;
			currtag +=1;
		}
	}

	/*2nd pass - Brute force. Loop through selected faces until there are no 'unbalanced' edges left (those with both vertices 'tmp.l' tag matching */
	while(ebalanced == 0){
		ebalanced = 1;
		for(eed=(EditEdge*)G.editMesh->edges.first; eed; eed = (EditEdge*)eed->next){
			if(eed->f&SELECT){
				if(eed->v1->tmp.l != eed->v2->tmp.l) /*unbalanced*/{
					if(eed->v1->tmp.l > eed->v2->tmp.l && eed->v2->tmp.l !=0) eed->v1->tmp.l = eed->v2->tmp.l; 
					else if(eed->v1 != 0) eed->v2->tmp.l = eed->v1->tmp.l; 
					ebalanced = 0;
				}
			}
		}
	}

	/*3rd pass, set all the edge flags (unnessecary?)*/
	for(eed=(EditEdge*)G.editMesh->edges.first; eed; eed = (EditEdge*)eed->next){
		if(eed->f&SELECT) eed->tmp.l = eed->v1->tmp.l;
	}

	for(eed=(EditEdge*)G.editMesh->edges.first; eed; eed=(EditEdge*)eed->next){
		if(eed->f&SELECT){
			if(allcollections->first){
				for(edgecollection = (Collection*)allcollections->first; edgecollection; edgecollection=(Collection*)edgecollection->next){
					if(edgecollection->index == eed->tmp.l){
						newedge = (CollectedEdge*)MEM_mallocN(sizeof(CollectedEdge), "collected edge");
						newedge->eed = eed;
						BLI_addtail(&(edgecollection->collectionbase), newedge);
						collectionfound = 1;
						break;
					}
					else collectionfound = 0;
				}
			}
			if(allcollections->first == NULL || collectionfound == 0){
				newcollection = (Collection*)MEM_mallocN(sizeof(Collection), "element collection");
				newcollection->index = eed->tmp.l;
				newcollection->collectionbase.first = 0;
				newcollection->collectionbase.last = 0;

				newedge = (CollectedEdge*)MEM_mallocN(sizeof(CollectedEdge), "collected edge");
				newedge->eed = eed;

				BLI_addtail(&(newcollection->collectionbase), newedge);
				BLI_addtail(allcollections, newcollection);
			}
		}

	}
}

static void 
freecollections(ListBase *allcollections)
{
	struct Collection *curcollection;

	for(curcollection = (Collection*)allcollections->first; curcollection; curcollection = (Collection*)curcollection->next)
		BLI_freelistN(&(curcollection->collectionbase));
	BLI_freelistN(allcollections);
}

static void collapse_edgeuvs(void)
{
	ListBase uvedges, uvverts, allcollections;
	wUVEdge *curwedge;
	wUVNode *curwnode;
	wUVEdgeCollect *collectedwuve, *newcollectedwuve;
	Collection *wuvecollection, *newcollection;
	int curtag, balanced, collectionfound= 0, vcount;
	float avg[2];

	if (!EM_texFaceCheck())
		return;

	uvverts.first = uvverts.last = uvedges.first = uvedges.last = allcollections.first = allcollections.last = NULL;

	build_weldedUVs(&uvverts);
	build_weldedUVEdges(&uvedges, &uvverts);

	curtag = 0;

	for(curwedge=(wUVEdge*)uvedges.first; curwedge; curwedge=(wUVEdge*)curwedge->next){
		curwedge->v1->f = curtag;
		curwedge->v2->f = curtag;
		curtag +=1;
	}

	balanced = 0;
	while(!balanced){
		balanced = 1;
		for(curwedge=(wUVEdge*)uvedges.first; curwedge; curwedge=(wUVEdge*)curwedge->next){
			if(curwedge->v1->f != curwedge->v2->f){
				if(curwedge->v1->f > curwedge->v2->f) curwedge->v1->f = curwedge->v2->f;
				else curwedge->v2->f = curwedge->v1->f;
				balanced = 0;
			}
		}
	}

	for(curwedge=(wUVEdge*)uvedges.first; curwedge; curwedge=(wUVEdge*)curwedge->next) curwedge->f = curwedge->v1->f;


	for(curwedge=(wUVEdge*)uvedges.first; curwedge; curwedge=(wUVEdge*)curwedge->next){
		if(allcollections.first){
			for(wuvecollection = (Collection*)allcollections.first; wuvecollection; wuvecollection=(Collection*)wuvecollection->next){
				if(wuvecollection->index == curwedge->f){
					newcollectedwuve = (wUVEdgeCollect*)MEM_callocN(sizeof(wUVEdgeCollect), "Collected Welded UV Edge");
					newcollectedwuve->uved = curwedge;
					BLI_addtail(&(wuvecollection->collectionbase), newcollectedwuve);
					collectionfound = 1;
					break;
				}

				else collectionfound = 0;
			}
		}
		if(allcollections.first == NULL || collectionfound == 0){
			newcollection = (Collection*)MEM_callocN(sizeof(Collection), "element collection");
			newcollection->index = curwedge->f;
			newcollection->collectionbase.first = 0;
			newcollection->collectionbase.last = 0;

			newcollectedwuve = (wUVEdgeCollect*)MEM_callocN(sizeof(wUVEdgeCollect), "Collected Welded UV Edge");
			newcollectedwuve->uved = curwedge;

			BLI_addtail(&(newcollection->collectionbase), newcollectedwuve);
			BLI_addtail(&allcollections, newcollection);
		}
	}

	for(wuvecollection=(Collection*)allcollections.first; wuvecollection; wuvecollection=(Collection*)wuvecollection->next){

		vcount = avg[0] = avg[1] = 0;

		for(collectedwuve= (wUVEdgeCollect*)wuvecollection->collectionbase.first; collectedwuve; collectedwuve = (wUVEdgeCollect*)collectedwuve->next){
			avg[0] += collectedwuve->uved->v1uv[0];
			avg[1] += collectedwuve->uved->v1uv[1];

			avg[0] += collectedwuve->uved->v2uv[0];
			avg[1] += collectedwuve->uved->v2uv[1];

			vcount +=2;

		}

		avg[0] /= vcount; avg[1] /= vcount;

		for(collectedwuve= (wUVEdgeCollect*)wuvecollection->collectionbase.first; collectedwuve; collectedwuve = (wUVEdgeCollect*)collectedwuve->next){
			for(curwnode=(wUVNode*)collectedwuve->uved->v1->nodes.first; curwnode; curwnode=(wUVNode*)curwnode->next){
				*(curwnode->u) = avg[0];
				*(curwnode->v) = avg[1];
			}
			for(curwnode=(wUVNode*)collectedwuve->uved->v2->nodes.first; curwnode; curwnode=(wUVNode*)curwnode->next){
				*(curwnode->u) = avg[0];
				*(curwnode->v) = avg[1];
			}
		}
	}

	free_weldedUVs(&uvverts);
	BLI_freelistN(&uvedges);
	freecollections(&allcollections);
}

static void build_weldedUVs(ListBase *uvverts)
{
	EditFace *efa;
	for(efa=(EditFace*)G.editMesh->faces.first; efa; efa=(EditFace*)efa->next){
		if(efa->v1->f1) append_weldedUV(efa, efa->v1, 0, uvverts);
		if(efa->v2->f1) append_weldedUV(efa, efa->v2, 1, uvverts);
		if(efa->v3->f1) append_weldedUV(efa, efa->v3, 2, uvverts);
		if(efa->v4 && efa->v4->f1) append_weldedUV(efa, efa->v4, 3, uvverts);
	}
}

static void append_weldedUV(EditFace *efa, EditVert *eve, int tfindex, ListBase *uvverts)
{
	wUV *curwvert, *newwvert;
	wUVNode *newnode;
	int found;
	MTFace *tf = (MTFace*)CustomData_em_get(&G.editMesh->fdata, efa->data, CD_MTFACE);

	found = 0;

	for(curwvert=(wUV*)uvverts->first; curwvert; curwvert=(wUV*)curwvert->next){
		if(curwvert->eve == eve && curwvert->u == tf->uv[tfindex][0] && curwvert->v == tf->uv[tfindex][1]){
			newnode = (wUVNode*)MEM_callocN(sizeof(wUVNode), "Welded UV Vert Node");
			newnode->u = &(tf->uv[tfindex][0]);
			newnode->v = &(tf->uv[tfindex][1]);
			BLI_addtail(&(curwvert->nodes), newnode);
			found = 1;
			break;
		}
	}

	if(!found){
		newnode = (wUVNode*)MEM_callocN(sizeof(wUVNode), "Welded UV Vert Node");
		newnode->u = &(tf->uv[tfindex][0]);
		newnode->v = &(tf->uv[tfindex][1]);

		newwvert = (wUV*)MEM_callocN(sizeof(wUV), "Welded UV Vert");
		newwvert->u = *(newnode->u);
		newwvert->v = *(newnode->v);
		newwvert->eve = eve;

		BLI_addtail(&(newwvert->nodes), newnode);
		BLI_addtail(uvverts, newwvert);

	}
}

static void append_weldedUVEdge(EditFace *efa, EditEdge *eed, ListBase *uvedges)
{
	wUVEdge *curwedge, *newwedge;
	int v1tfindex, v2tfindex, found;
	MTFace *tf = (MTFace*)CustomData_em_get(&G.editMesh->fdata, efa->data, CD_MTFACE);

	found = 0;

	if(eed->v1 == efa->v1) v1tfindex = 0;
	else if(eed->v1 == efa->v2) v1tfindex = 1;
	else if(eed->v1 == efa->v3) v1tfindex = 2;
	else /* if(eed->v1 == efa->v4) */ v1tfindex = 3;

	if(eed->v2 == efa->v1) v2tfindex = 0;
	else if(eed->v2 == efa->v2) v2tfindex = 1;
	else if(eed->v2 == efa->v3) v2tfindex = 2;
	else /* if(eed->v2 == efa->v4) */ v2tfindex = 3;

	for(curwedge=(wUVEdge*)uvedges->first; curwedge; curwedge=(wUVEdge*)curwedge->next){
		if(curwedge->eed == eed && curwedge->v1uv[0] == tf->uv[v1tfindex][0] && curwedge->v1uv[1] == tf->uv[v1tfindex][1] && curwedge->v2uv[0] == tf->uv[v2tfindex][0] && curwedge->v2uv[1] == tf->uv[v2tfindex][1]){
			found = 1;
			break; //do nothing, we don't need another welded uv edge
		}
	}

	if(!found){
		newwedge = (wUVEdge*)MEM_callocN(sizeof(wUVEdge), "Welded UV Edge");
		newwedge->v1uv[0] = tf->uv[v1tfindex][0];
		newwedge->v1uv[1] = tf->uv[v1tfindex][1];
		newwedge->v2uv[0] = tf->uv[v2tfindex][0];
		newwedge->v2uv[1] = tf->uv[v2tfindex][1];
		newwedge->eed = eed;

		BLI_addtail(uvedges, newwedge);
	}
}

static void build_weldedUVEdges(ListBase *uvedges, ListBase *uvverts)
{
	wUV *curwvert;
	wUVEdge *curwedge;
	EditFace *efa;

	for(efa=(EditFace*)G.editMesh->faces.first; efa; efa=(EditFace*)efa->next){
		if(efa->e1->f1) append_weldedUVEdge(efa, efa->e1, uvedges);
		if(efa->e2->f1) append_weldedUVEdge(efa, efa->e2, uvedges);
		if(efa->e3->f1) append_weldedUVEdge(efa, efa->e3, uvedges);
		if(efa->e4 && efa->e4->f1) append_weldedUVEdge(efa, efa->e4, uvedges);
	}


	//link vertices: for each uvedge, search uvverts to populate v1 and v2 pointers
	for(curwedge=(wUVEdge*)uvedges->first; curwedge; curwedge=(wUVEdge*)curwedge->next){
		for(curwvert=(wUV*)uvverts->first; curwvert; curwvert=(wUV*)curwvert->next){
			if(curwedge->eed->v1 == curwvert->eve && curwedge->v1uv[0] == curwvert->u && curwedge->v1uv[1] == curwvert->v){
				curwedge->v1 = curwvert;
				break;
			}
		}
		for(curwvert=(wUV*)uvverts->first; curwvert; curwvert=(wUV*)curwvert->next){
			if(curwedge->eed->v2 == curwvert->eve && curwedge->v2uv[0] == curwvert->u && curwedge->v2uv[1] == curwvert->v){
				curwedge->v2 = curwvert;
				break;
			}
		}
	}
}

static void free_weldedUVs(ListBase *uvverts)
{
	wUV *curwvert;
	for(curwvert = (wUV*)uvverts->first; curwvert; curwvert=(wUV*)curwvert->next) BLI_freelistN(&(curwvert->nodes));
	BLI_freelistN(uvverts);
}

int 
BL_CollapseEdges(void)
{
	EditVert *eve;
	EditEdge *eed;

	ListBase allcollections;
	CollectedEdge *curredge;
	Collection *edgecollection;

	int totedges, groupcount, mergecount,vcount;
	float avgcount[3];

	allcollections.first = 0;
	allcollections.last = 0;

	mergecount = 0;

	if(multires_test()) return 0;

	build_edgecollection(&allcollections);
	groupcount = BLI_countlist(&allcollections);


	for(edgecollection = (Collection*)allcollections.first; edgecollection; edgecollection = (Collection*)edgecollection->next){
		totedges = BLI_countlist(&(edgecollection->collectionbase));
		mergecount += totedges;
		avgcount[0] = 0; avgcount[1] = 0; avgcount[2] = 0;

		vcount = 0;

		for(curredge = (CollectedEdge*)edgecollection->collectionbase.first; curredge; curredge = (CollectedEdge*)curredge->next){
			avgcount[0] += ((EditEdge*)curredge->eed)->v1->co[0];
			avgcount[1] += ((EditEdge*)curredge->eed)->v1->co[1];
			avgcount[2] += ((EditEdge*)curredge->eed)->v1->co[2];

			avgcount[0] += ((EditEdge*)curredge->eed)->v2->co[0];
			avgcount[1] += ((EditEdge*)curredge->eed)->v2->co[1];
			avgcount[2] += ((EditEdge*)curredge->eed)->v2->co[2];

			vcount +=2;
		}

		avgcount[0] /= vcount; avgcount[1] /=vcount; avgcount[2] /= vcount;

		for(curredge = (CollectedEdge*)edgecollection->collectionbase.first; curredge; curredge = (CollectedEdge*)curredge->next){
			VECCOPY(((EditEdge*)curredge->eed)->v1->co,avgcount);
			VECCOPY(((EditEdge*)curredge->eed)->v2->co,avgcount);
		}

		if (EM_texFaceCheck()) {
			/*uv collapse*/
			for(eve=(EditVert*)G.editMesh->verts.first; eve; eve=(EditVert*)eve->next) eve->f1 = 0;
			for(eed=(EditEdge*)G.editMesh->edges.first; eed; eed=(EditEdge*)eed->next) eed->f1 = 0;
			for(curredge = (CollectedEdge*)edgecollection->collectionbase.first; curredge; curredge = (CollectedEdge*)curredge->next){
				curredge->eed->v1->f1 = 1;
				curredge->eed->v2->f1 = 1;
				curredge->eed->f1 = 1;
			}
			collapse_edgeuvs();
		}

	}
	freecollections(&allcollections);
	removedoublesflag(1, 0, MERGELIMIT);
	/*get rid of this!*/
	countall();
	DAG_object_flush_update(G.scene, G.obedit, OB_RECALC_DATA);

	return mergecount;
}

VOID
BL_MeshUnselectAll( Mesh* pMesh )
{
	MVert* pV = NULL;
	MEdge* pE = NULL;
	MFace* pF = NULL;

	for( DWORD i=0; i < pMesh->totedge; i++ )
	{
		pE = &pMesh->medge[i];
		pE->flag &= ~SELECT;
	}

	for( DWORD i=0; i < pMesh->totvert; i++ )
	{
		pV = &pMesh->mvert[i];
		pV->flag &= ~SELECT;
	}

	for( DWORD i=0; i < pMesh->totface; i++ )
	{
		pF = &pMesh->mface[i];
		pF->flag &= ~SELECT;
	}
}

D3DXVECTOR3
BL_CalcCenter( Mesh* mesh, D3DXVECTOR3* vMiddle )
{
	MVert *pv = NULL;
	float minx = 0, miny = 0, minz = 0, maxx = 0, maxy = 0, maxz = 0;

	for( int i=0; i < mesh->totvert; i++ )
	{
		pv = &mesh->mvert[i];
		if( pv->co[0] < minx ) minx = pv->co[0];
		if( pv->co[0] > maxx ) maxx = pv->co[0];
		if( pv->co[1] < miny ) miny = pv->co[1];
		if( pv->co[1] > maxy ) maxy = pv->co[1];
		if( pv->co[2] < minz ) minz = pv->co[2];
		if( pv->co[2] > maxz ) maxz = pv->co[2];
	}

	if( vMiddle )
	{
		vMiddle->x = ( minx < 0 ? (maxx - minx) / 2.f : (maxx + minx) / 2.f );
		vMiddle->y = ( miny < 0 ? (maxy - miny) / 2.f : (maxy + miny) / 2.f );
		vMiddle->z = ( minz < 0 ? (maxz - minz) / 2.f : (maxz + minz) / 2.f );
	}
	return D3DXVECTOR3( (minx + maxx) / 2.f, (miny + maxy) / 2.f, (minz + maxz) / 2.f );
}

VOID
BL_ResizeMesh( Object* ob )
{
	Mesh* pMesh; 
	MVert* pV;

	float min, max;
	

	
	for( Object* pOb = ob; pOb; pOb = (Object*)pOb->id.next )
	{
		pMesh = (Mesh*)pOb->data;
		min = max = 0;
		for( int i=0; i < pMesh->totvert; i++ )
		{
			pV = &pMesh->mvert[i];
			if( pV->co[0] < min ) min = pV->co[0];
			if( pV->co[0] > max ) max = pV->co[0];
			if( pV->co[1] < min ) min = pV->co[1];
			if( pV->co[1] > max ) max = pV->co[1];
			if( pV->co[2] < min ) min = pV->co[2];
			if( pV->co[2] > max ) max = pV->co[2];
		}

		for( DWORD i=0; i < pMesh->totvert; i++ )
		{
			pV = &pMesh->mvert[i];
			for( int j=0; j < 3; j++ ) pV->co[j] /= (max / 2.f);
		}

	}
}