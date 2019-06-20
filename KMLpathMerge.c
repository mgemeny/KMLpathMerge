#include<stdio.h>
#include <fcntl.h>
#include <math.h>
#include <stdlib.h>


/* C program to merge a number of poly-lines (aka paths) into a single path. */
/* The paths are assumed to be arranged into the appropriate order in the KML. */
/* This program will find the best-fit between adjacent segments.*/
/* In Google Earth it may not be apparent that paths are drawn in some particular direction. */
/* For example, east to west, or west to east. */
/* The goal of this utility program is to process segments forward or backward as  */
/* appropriate to create a single path. */
/* The appropriate processing direction of the first segment is established by the best */
/* fit with the second segment. */
/* The appropriate processing direction of all subsequent segments is a best fit with */
/* the last end of the previous section processed.*/
/*  */
/*  */
/*  */

/* By Michael (Mike) Gemeny */

/* Version history */


/* Contributors include */
/*  Aayush Chaturvedi */
/*  Michael (Mike) Gemeny */
/*  Norwin Malmberg */
/*  Fred Hayes */
/*  */
/*  */
/*                                Globals */

/*  */
/*  */
/* We use a lot of them here to make the rest of the code easier to read. */
/* Yes, we know it makes it a bit harder to reuse code. */
/*  */
char FileName[256];/* The name of the input KML file */
char OutFileName[256];/* The name of the output KML file */
extern char *optarg;
int sf,sp,sh,sd,sc,sw,st,sl,su,so,sn,sm,sr,se,sv = 0; /* Integers used as booleans for various command line switches, all start false */

FILE *kml; /* This will be our file handle for the input KML file */
FILE *outfile; /* This will be out file handle for the output KML file */
int startcoord;	  /* The position in the KML just after the whitespace before the first coord */
int endcoord;	  /* The position in the file just after the space at the end of the last coord */
int endifreverse; /* The position just after reading the first physical coordinate */
double lat = 0;   /* Working variable for Lattitude */
double lon = 0;	  /* Working variable for Longitude */
int position;	/* everyone's working variable for ftell file position */
double seglength;
double pathlength = 0; /* Everyone's path length */
double tpathlength = 0; /* Total path length used in conjunction with pathlength*/
double slackratio = 1;
double lat1 = 53.32055555555556; 
double long1 = -1.7297222222222221; 
double lat2 = 53.31861111111111; 
double long2 = -1.6997222222222223; 
double lat3;	/* return values */
double long3;	/* return values */
double latA;	/* Used to hold the beginning of the first segments */
double longA;	/* Used to hold the beginning of the first segment */
double latB;	/* Used to hold the end of the first segment */
double longB;	/* Used to hold the end of the first segment */
double latC;	/* Used to hold the beginning of the second segment */
double longC;	/* Used to hold the beginning of the second segment */
double latD;	/* Used to hold the end of the second segment */
double longD;	/* Used to hold the end of the second segment */
double AtoC;
double AtoD;
double shortestA;
double BtoC;
double BtoD;
double shortestB;
int firstSegementGetsFlipped;


double elevation = 1;
int i,ch;
int count; /* used by Tally to count the segments */


/*  */
/*  */
/*                                The help is here for convenient reference */

/*  */
/*  */

void checkhelp( int argc )
{
	if ( argc == 1 ) 
	{
	printf("USAGE\n");
	printf("KMLpathMerge -I <InputFileName> -O <OutputFileName> [-v]\n");
	printf("\n");
	printf("Where -v is verbose\n");
	printf("\n");
	printf("This program merges the various paths found in the InputFile producing OutputFile.\n");
	printf("The path segments in the input file are assumed to be in proper order.\n");
	printf("This can be done using Google Earth with relative ease.\n");
	printf("For best results, paths should no be bundled into folders within the KML.\n");
	printf("The goal of this utility is to find a best fit between the end points of the adjacent segments\n");
	printf("IE. to flip segments as appropriate for them to properly be appended together.\n");
	printf("\n");
	exit (0);

	}
}
/* Some notes, reminders, and explanations. */
/* We are using the Haversine Formula below, and we are finding a mid point with the lat/lon average method. */
/* These suit our needs for the precession vs processing power trade off. */
/* For your application, you may want to derive from Haversine an equation for midpoint. */
/* You may also want to use Vincenty's formulae to represent the earth as an oblate spheroid, */
/* and perhaps derive a midpoint equation from Vincenty's formula. */
/* The code should be well enough organized for you to drop in what ever equations you would like. */
/* For our application, we have established that the method used here is sufficient, YMMV */
/*  */
/* A bit about KMZs and KMLs */
/* The code expects a KML of the paths on the earth to be processed. */
/* A KMZ is often just a zipped KML or set of KMLs */
/* On some systems you may need to rename your KMZ */
/* with a file extention of .zip in order to unzip it. */
/* KML stands for 'Keyhole Markup Language'  */
/* Before Google purchased 'Google Earth' it was called */
/* 'Keyhole Earth Viewer', hence the name. */
/* As a 'Markup Language' (Think 'HTML') you can explore */
/* it with your favorite text editor, although some editors work  */
/* better than others. */
/* A path in a KML is represented by a list of Lon, Lat, Elevation coordinates. */
/*  */
/*  */
/*  */
/*                    Functions which do not read the file or move the file pointer */
/*                           Generally they are math or display functions */
/*  */
/*  */

/* C program to calculate Distance */
/* Between Two Points on Earth */
/* This code is contributed */
/* by Aayush Chaturvedi */
/* and Mike Gemeny */
 

/* Utility function for */
/* converting degrees to radians */


double toRadians(double degree) 
{ 
    /* cmath library in C++ */
    /* defines the constant */
    /* M_PI as the value of */
    /* pi accurate to 1e-30 */

	double one_deg = ((M_PI) /  180.0);
   
	return (one_deg * degree); 
}
  
/* Utility function to find */
/* the distance between two points */
/* using the Haversine Formula */

double distance(double lat1, double long1, double lat2, double long2) 
{ 


    /* Convert the latitudes */
    /* and longitudes */
    /* from degree to radians  */
    lat1 = toRadians(lat1); 
    long1 = toRadians(long1); 
    lat2 = toRadians(lat2); 
    long2 = toRadians(long2); 


      
    /* Haversine Formula */
    double dlong = long2 - long1; 
    double dlat = lat2 - lat1; 
  
    double ans = pow(sin(dlat / 2), 2) +  
                          cos(lat1) * cos(lat2) *  
                          pow(sin(dlong / 2), 2); 
 
    ans = 2 * asin(sqrt(ans)); 
  
    /* Radius of Earth in */
    /* Kilometers, R = 6371 */
    /* Use R = 3956 for miles */
    double R = 6371; 
      
    /* Calculate the result */
    ans = ans * R; 
    seglength = ans;
    return ans; 
} ;




/*  */
/*  */
/*  */
/*                                  Functions which are low level file manipulation */
/*                         They always treat the file without regard to the -r (reverse) switch */
/*  */
/*  */
/*  */

/* Utility function to position */
/* a file just after a given string */
/*  */
int searchto(char *search, FILE *sf ) 
	{ 
    	/*  */
    	/*  */
    	/*  */
    	/*  */
	char buff[1000];
	char *stripfirst;
	char readvar[4];
	int bufflength;
	/* Special case that it's just one character being searched for */
	if (strlen(search) == 1) {
		buff[0] = '\0';     /* install the end zero bytes */
		buff[1] = '\0';
		while ( strcmp(buff,search) && !feof(sf))
			{
			fread(buff,1,1,sf);           /* read the next character */
			}
		return(0);

	}
	stripfirst = buff;               /* points to our buffer */
	*stripfirst++;                   /* but skip the first character */
	fread(buff,1,strlen(search),sf); /* prime the pump */
	buff[strlen(search)] = '\0';     /* install the ending zero byte */
	while ( strcmp(buff,search) && !feof(sf)) 
		{
		/*printf ("buff-%s-search-%s-" ,buff ,search);*/
		strcpy(buff,stripfirst);         /* now slide the buffer */
		fread(readvar,1,1,sf);           /* read the next character */
		bufflength = strlen(buff);       /*  */
		buff[bufflength] = readvar[0];   /* and add it to the end */
		buff[(bufflength + 1)] = '\0';   /* and set the new end marker */
		}
	return(0);
	}



void backtillspace(FILE *sf) /* Positions backward just into prior space */
	{
	char buff[4];
	buff[1] = '\0';     /* install the end zero byte */
	int position;
	position = ftell(sf);
	fseek( sf, -2, SEEK_CUR );
        fread(buff,1,1,sf);           /* prime the pump */
	/* while a linefeed, tab, return, or space keep backing up*/
	while ( strcmp ( buff, "\n" ) == 0 || strcmp ( buff, "\t" ) == 0 || strcmp ( buff, "\r" ) == 0 || strcmp ( buff, " " ) == 0 )
		{
		fseek( sf, -2, SEEK_CUR ); /* keep backing up */
        	fread(buff,1,1,sf);
		}
	fread(buff,1,1,sf); /* move forward one character into the whitespace */
	}

int back1coord(FILE *sf)
	{
	char buff[4];
	buff[1] = '\0';     /* install the end zero byte */
	int position;
	position = ftell(sf);
	fseek( sf, -2, SEEK_CUR );
        fread(buff,1,1,sf);           /* prime the pump */
	/* while not a linefeed, tab, return, or space */
	while (  strcmp ( buff, "\n" ) != 0 && strcmp ( buff, "\t" ) != 0 && strcmp ( buff, "\r" ) != 0 && strcmp ( buff, " " ) != 0 )
		{
		fseek( sf, -2, SEEK_CUR );
        	fread(buff,1,1,sf);
		}
	}

int readacoord(FILE *sf)	/* Results are returned in the global lat and lon variables */
	{
	int i;
	i = fscanf(sf,"%lf",&lon);	/* For what ever reason, the Longitude seems to come first in a KML */
	i = fscanf(sf,",%lf",&lat);	/* and then the Latitude */
	searchto(" ",sf);		/* and skip the Elevation */
	}



int Writeacoord(FILE *sf)       /*  Write a Coordinate with values in global lat and lon as readacoord above*/
	{
	fprintf(sf,"%0.14lf",lon);		/* First the Longitude */
	fprintf(sf,",");			/* A comma */
	fprintf(sf,"%0.14lf",lat);		/* Then the Latitude (for what ever reason its backward) */
	fprintf(sf,",0 ");			/* And another comma, and zero for elivation, then a space */
	}


/*  */
/*  */
/*                           Functions which are higher level file manipulation  */
/*                             They always respect the -r (reverse) switch       */
/*                             These do rely more on initialization by main      */   
/*  */

/* read first coordinate */
int readfirstcoord(FILE *sf)
	{
	if (sr == 1) /* We're in reverse */
		{
		fseek( sf, endcoord, SEEK_SET ); /* go back to the physical end of the coordinates */
		back1coord(sf);
		readacoord(sf);
		}
	else         /* We're reading forward */
		{
		fseek( sf, startcoord, SEEK_SET ); /* go to the beginning of the coordinates */
		readacoord(sf);
		}
	}

/* read next coordinate */
int readnextcoord(FILE *sf)
	{
	if (sr == 1) /* We're in reverse */
		{
		back1coord(sf);
		back1coord(sf);
		readacoord(sf);
		}
	else         /* We're reading forward */
		{
		readacoord(sf);
		}
	}


/* check for last coordinate */
int checklastcoord(FILE *sf)
	{
	position = ftell(sf);
	if (sr == 1) /* We're in reverse */
		{
		if (position == endifreverse) return 1;
		return 0;
		}
	else         /* We're reading forward */
		{
		if (position == endcoord) return 1;
		return 0;
		}
	}


/*  */








/* Main ************************************************************** */
/* Driver Code; */ 
int main(int argc,char **argv) 
{ 
checkhelp( argc );	


/* First, lets go collect all of the command line switches */
 while ((ch = getopt(argc,argv,"I:O:v")) != -1) {
        switch(ch) {
        case 'I':
            sf = 1;	
            strcpy(FileName,optarg);
            break;

        case 'O':
            sp = 1;
            strcpy(OutFileName,optarg);
            break;

	case 'v':
            sv = 1;
            break;

        default:
            printf("*Unknown option %c\n",ch);
            return 1;
        }
    }
/* now lets try to open the input KML */

     if ((kml = fopen(FileName,"rb")) <0) 
	{
         printf("*Cannot open input file %s\n",FileName);
	 exit (0);
        }

/* now lets try to open the output KML */

     if ((outfile = fopen(OutFileName,"w")) <0) 
	{
         printf("*Cannot open output file %s\n",OutFileName);
	 exit (0);
        }


searchto("<Document>",kml);

searchto("<Placemark>",kml);

searchto("<name>",kml);

searchto("<coordinates>",kml);

/* Find the exact beginning and end of the coordinates for the first path segment and remember them */
readacoord(kml);
endifreverse = ftell(kml); /* set the marker for the end of the coordinates if reading in reverse*/
back1coord(kml);
startcoord = ftell(kml); /* set the marker for the beginning of the coordinates */
searchto("<",kml);
backtillspace(kml);
endcoord = ftell(kml); /* set the marker for the end of the coordinates */
fseek( kml, startcoord, SEEK_SET ); /* go back to the beginning of the coordinates */

/* Next we populate A,B,C,D coordinates to find out if the first segment needs to be processed in reverse order */
readacoord(kml);
latA = lat;	
longA = lon;	

fseek( kml, endcoord, SEEK_SET ); /* go back to the physical end of the coordinates for the first path segment */
back1coord(kml);
readacoord(kml);
latB = lat;	
longB = lon;	

/* Now we search for the second set of coordinates */
searchto("<coordinates>",kml);
 if feof(kml)
	{
	printf("We didn't find a second set of coordinates!\n");
	printf("Processing aborted with no further output!\n");
	fclose(kml);
	fclose(outfile);
	exit (0);
	}
readacoord(kml); /* This should read the first coord of the second set */
latC = lat;	
longC = lon;
searchto("<",kml); /* This show find the end of the second set of coords */
backtillspace(kml);
back1coord(kml);
readacoord(kml); /* This should read the last coord of the second set */
latD = lat;	
longD = lon;	
	
fseek( kml, startcoord, SEEK_SET ); /* go back to the beginning of the first set of coordinates */

/* Now we can start doing some math to figure out if the first segment needs to be reversed */
AtoC = distance(latA, longA, latC, longC); /* check the begining of the first segment against the begining and end of the second */
AtoD = distance(latA, longA, latD, longD);
 if (AtoC <= AtoD)
	{
	shortestA = AtoC; /* shortestA is because we do not really care if the second segemnt needs reversal at this point */
	}
 else
	{
	shortestA = AtoD;
	}

BtoC = distance(latB, longB, latC, longC); /* Check the end of the first segment against the begining and end of the second */
BtoD = distance(latB, longB, latD, longD);
 if (BtoC <= BtoD)
	{
	shortestB = BtoC; /* shortestB is because we do not really care if the second segemnt needs reversal at this point */
	}
 else
	{
	shortestB = BtoD;
	}

if (shortestB <= shortestA) /* then no reversal of the first segment is required */
	{
	sr = 0; /* No reverse switch */
	if (sv = 1) printf ("First segment goes forward\n");
	}
	else
	{
	sr = 1; /* set the reverse switch */
	if (sv = 1) printf ("First segment goes Reverse\n");
	}

/* Next we start to construct the output KML */
fprintf (outfile,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
fprintf (outfile,"<kml xmlns=\"http://www.opengis.net/kml/2.2\" xmlns:gx=\"http://www.google.com/kml/ext/2.2\" xmlns:kml=\"http://www.opengis.net/kml/2.2\" xmlns:atom=\"http://www.w3.org/2005/Atom\">\n");
fprintf (outfile,"<Document>\n");
fprintf (outfile,"\t<name>%s</name>\n",OutFileName);
fprintf (outfile,"\t<Style id=\"inline\">\n");
fprintf (outfile,"\t\t<LineStyle>\n");
fprintf (outfile,"\t\t\t<color>ff0000ff</color>\n");
fprintf (outfile,"\t\t\t<width>2</width>\n");
fprintf (outfile,"\t\t</LineStyle>\n");
fprintf (outfile,"\t\t<PolyStyle>\n");
fprintf (outfile,"\t\t\t<fill>0</fill>\n");
fprintf (outfile,"\t\t</PolyStyle>\n");
fprintf (outfile,"\t</Style>\n");
fprintf (outfile,"\t<StyleMap id=\"inline0\">\n");
fprintf (outfile,"\t\t<Pair>\n");
fprintf (outfile,"\t\t\t<key>normal</key>\n");
fprintf (outfile,"\t\t\t<styleUrl>#inline</styleUrl>\n");
fprintf (outfile,"\t\t</Pair>\n");
fprintf (outfile,"\t\t<Pair>\n");
fprintf (outfile,"\t\t\t<key>highlight</key>\n");
fprintf (outfile,"\t\t\t<styleUrl>#inline1</styleUrl>\n");
fprintf (outfile,"\t\t</Pair>\n");
fprintf (outfile,"\t\t</StyleMap>\n");
fprintf (outfile,"\t<Style id=\"inline1\">\n");
fprintf (outfile,"\t\t<LineStyle>\n");
fprintf (outfile,"\t\t\t<color>ff0000ff</color>\n");
fprintf (outfile,"\t\t\t<width>2</width>\n");
fprintf (outfile,"\t\t</LineStyle>\n");
fprintf (outfile,"\t\t<PolyStyle>\n");
fprintf (outfile,"\t\t\t<fill>0</fill>\n");
fprintf (outfile,"\t\t</PolyStyle>\n");
fprintf (outfile,"\t</Style>\n");
fprintf (outfile,"\t<Placemark>\n");
fprintf (outfile,"\t\t<name>MergedPath</name>\n");
fprintf (outfile,"\t\t<styleUrl>#inline0</styleUrl>\n");
fprintf (outfile,"\t\t<LineString>\n");
fprintf (outfile,"\t\t\t<tessellate>1</tessellate>\n");
fprintf (outfile,"\t\t\t<coordinates>\n");
fprintf (outfile,"\t\t\t\t"); /* And now the output file is ready to receive space seperated coordinates */






/*  */
/*  */
/*                        End of Initilization  */
/*                          Lets get to work    */
/*  */
/*  */


/* The input file is already position with markers set and we already know if the first segment is backward */
/* The Output file is ready for coordinates */

while ( !feof(kml))	/* While not EOF on the input file, keep processing segments*/
	{
	readfirstcoord(kml);
	Writeacoord(outfile);
	while (!checklastcoord(kml))	/* While not end of coordinates for this segment, keep copying them */
		{
		readnextcoord(kml);
		Writeacoord(outfile);
		}
	latC = lat;	/* Now we are just going to use lat/lon C to keep track of where we left off for positioning the next segment */
	longC = lon;

			/* set up for the next segment */

	searchto("</coordinates>",kml); /* Find the end of the current set of coords */
	searchto("<coordinates>",kml);  /* Find the begining of the next set of coords or EOF*/
	readacoord(kml);
	endifreverse = ftell(kml); /* set the marker for the end of the coordinates if reading in reverse*/
	back1coord(kml);
	startcoord = ftell(kml); /* set the marker for the beginning of the coordinates */
	searchto("<",kml);
	backtillspace(kml);
	endcoord = ftell(kml); /* set the marker for the end of the coordinates */
	fseek( kml, startcoord, SEEK_SET ); /* go back to the beginning of the coordinates */
	/* Next we populate A,B coordinates to find out if the next segment needs to be processed in reverse order */
	readacoord(kml);
	latA = lat;	
	longA = lon;	
	fseek( kml, endcoord, SEEK_SET ); /* go back to the physical end of the coordinates for the next path segment */
	back1coord(kml);
	readacoord(kml);
	latB = lat;	
	longB = lon;	
	AtoC = distance(latA, longA, latC, longC); /* check the begining of this next segment against the end of the last segment */
	BtoC = distance(latB, longB, latC, longC); /* Check the end of this next segment against the end of the last segment */
	if (AtoC <= BtoC) /* then no reversal of the next segment is required */
		{
		sr = 0; /* No reverse switch */
		if ((sv = 1) && (!feof(kml))) printf ("Next segment goes forward\n");
		}
		else
		{
		sr = 1; /* set the reverse switch */
		if ((sv = 1) && (!feof(kml))) printf ("Next segment goes reverse\n");
		}

	/* Let 'r rip! */

	
	}





fprintf (outfile,"\n"); /* The output file should already have a trailing space after the last coordinate */
fprintf (outfile,"\t\t\t</coordinates>\n"); /* Close up the coordinates section */
fprintf (outfile,"\t\t</LineString>\n"); /* Close up the LineString section */
fprintf (outfile,"\t</Placemark>\n"); /* Close up the Placemark section */
fprintf (outfile,"</Document>\n"); /* Close up the Document */
fprintf (outfile,"</kml>\n"); /* And at long last close up the KML section of the output XML */
fprintf (outfile,"\n");



fclose(kml);
fclose(outfile);
exit (0);

};

  


