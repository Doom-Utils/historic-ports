#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

char str[500][250];

int main()
  {
  FILE *f;
  int maxstr;

  //read in files
  f=fopen("strings.h","r");

  maxstr=0;
  while (!feof(f))
    {
    char tempstr[250];

    fgets(tempstr,250,f);
    if (strncmp(tempstr,"extern char*",strlen("extern char*"))==0)
      {
      sscanf(tempstr,"extern char *  %s",str[maxstr]);
      int i=0; while (str[maxstr][i]!=0)
        {
        if (str[maxstr][i]==';')
          str[maxstr][i]=0;
        i++;
        }
      printf("%s\n",str[maxstr]);
      maxstr++;
      }
    }
  fclose(f);

  //now write out strings.c
  f=fopen("strings.c","w");
  fprintf(f,"#include <stdio.h>\n");
  fprintf(f,"#include <stdlib.h>\n");
  fprintf(f,"#include \"doomstat.h\"\n");
  fprintf(f,"#include \"d_englsh.h\"\n");
  fprintf(f,"#include \"d_french.h\"\n");
  fprintf(f,"#include \"d_germ1.h\"\n");
  fprintf(f,"#include \"d_germ2.h\"\n");
  fprintf(f,"#include \"d_turk.h\"\n");
  fprintf(f,"#include \"d_swe.h\"\n");
  fprintf(f,"#include \"d_span.h\"\n");
  fprintf(f,"#include \"d_dutch.h\"\n");

  fprintf(f,"//first define all the variables...\n");
  for (int i=0;i<maxstr;i++)
    {
    fprintf(f,"char* %s;\n",str[i]);
    fprintf(f,"char* eng%s=ENG%s;\n",str[i],str[i]);
    fprintf(f,"char* frn%s=FRN%s;\n",str[i],str[i]);
    fprintf(f,"char* gr1%s=GR1%s;\n",str[i],str[i]);
    fprintf(f,"char* gr2%s=GR2%s;\n",str[i],str[i]);
    fprintf(f,"char* trk%s=TRK%s;\n",str[i],str[i]);
    fprintf(f,"char* swe%s=SWE%s;\n",str[i],str[i]);
    fprintf(f,"char* spa%s=SPA%s;\n",str[i],str[i]);
    fprintf(f,"char* dut%s=DUT%s;\n",str[i],str[i]);
    }

  fprintf(f,"\n\n//now the init function...\n");
  fprintf(f,"void initstrings() {\n");
  fprintf(f,"switch (language){\n");

  fprintf(f,"  case unknown:\n");
  fprintf(f,"  case english:\n");
  for (int i=0;i<maxstr;i++)
    fprintf(f,"    %s=eng%s;\n",str[i],str[i]);
  fprintf(f,"    break;\n\n");

  fprintf(f,"  case french:\n");
  for (int i=0;i<maxstr;i++)
    fprintf(f,"    %s=frn%s;\n",str[i],str[i]);
  fprintf(f,"    break;\n\n");

  fprintf(f,"  case german1:\n");
  for (int i=0;i<maxstr;i++)
    fprintf(f,"    %s=gr1%s;\n",str[i],str[i]);
  fprintf(f,"  break;\n\n");

  fprintf(f,"  case german2:\n");
  for (int i=0;i<maxstr;i++)
    fprintf(f,"    %s=gr2%s;\n",str[i],str[i]);
  fprintf(f,"  break;\n\n");

  fprintf(f,"  case turkish:\n");
  for (int i=0;i<maxstr;i++)
    fprintf(f,"    %s=trk%s;\n",str[i],str[i]);
  fprintf(f,"  break;\n\n");

  fprintf(f,"  case swedish:\n");
  for (int i=0;i<maxstr;i++)
    fprintf(f,"    %s=swe%s;\n",str[i],str[i]);
  fprintf(f,"  break;\n\n");

  fprintf(f,"  case spanish:\n");
  for (int i=0;i<maxstr;i++)
    fprintf(f,"    %s=spa%s;\n",str[i],str[i]);
  fprintf(f,"  break;\n\n");

  fprintf(f,"  case dutch:\n");
  for (int i=0;i<maxstr;i++)
    fprintf(f,"    %s=dut%s;\n",str[i],str[i]);
  fprintf(f,"  break;\n\n");

  fprintf(f,"}\n}\n\n");

  fprintf(f,"\n\n//finally, dehacked string searcher...\n");
  fprintf(f,"int searchstring(char *origtext, char *newtext) {\n");
  for (int i=0;i<maxstr;i++)
    {
    fprintf(f,"if (strcmp(origtext,eng%s)==0)\n",str[i]);
    fprintf(f,"   {%s=(char *)malloc(strlen(newtext)+1); strcpy(%s,newtext); return 1;}\n",str[i],str[i]);
    }
  fprintf(f,"return 0;}");

  fclose(f);

  return 666;
  }
