#include <string.h>
#include <stdio.h>

#include "esp_err.h"
#include "sht31arp.h"
#include "sht31dis.h"
#include "ads1115.h"
#include "config.h"


#define MAX_ARG 5
char* argv[MAX_ARG+1];
int argc;





typedef struct t_cmd_record{
	char cmd_str[10];
	char *((*cmd_func)(int argc, char *argv[],char * output_str));
}t_cmd_record;


char* cmd_1(int argc, char *argv[], char* output_str)
{
    strcpy(output_str,"cmd_1");
    return output_str;
}


char* cmd_2(int argc, char *argv[], char* output_str)
{
    int total_print=0;
    int cur_print=0;
    int i=0;
    total_print=sprintf(output_str,"cmd_2");
    for (i=0;i<argc;i++)
    {
        cur_print=sprintf(output_str+total_print," %s",argv[i]);
        total_print+=cur_print;
    }
    return output_str; 
}


char* cmd_read_temp(int argc, char *argv[], char* output_str)
{
	float temp = sht31arp_read_temp();
	sprintf(output_str,"%2.1f",temp);
    return output_str;
}

char* cmd_read_humid(int argc, char *argv[], char* output_str)
{
	float humid = sht31arp_read_humid();
	sprintf(output_str,"%2.1f",humid);
    return output_str;
}

char* cmd_read_th(int argc, char *argv[], char* output_str)
{
	float temp,humid;
	sht31_read(&temp,&humid);
	sprintf(output_str,"T=%2.1f H=%3.1f",temp,humid);
	return output_str;
}

char* cmd_set_cfg(int argc, char *argv[], char* output_str)
{
	esp_err_t err;
	
	err = config_set_key_str(argv[0],argv[1]);
	if (err==ESP_OK) {
		strcpy(output_str,"set ok");
	} else {
		strcpy(output_str,"set fail");
	}
    return output_str;
}

char* cmd_get_cfg(int argc, char *argv[], char* output_str)
{
	esp_err_t err;
	
	err = config_get_key_str(argv[0],output_str,"key not set");
	
	if (err!=ESP_OK) {
		strcpy(output_str,"get fail!");
		
	}
	
	return output_str;
}

char* cmd_read_xadc(int argc, char *argv[], char* output_str)
{
	float val[4];
	int channel=0;
	val[0]=val[1]=val[2]=val[4]=0;
	ads1115_read(0,&val[0]);
	ads1115_read(3,&val[1]);
	sprintf(output_str,"xADC %1.3f,%1.3f",val[0],val[1]);
	return output_str;
}



t_cmd_record cmd_table[] = {{"cmd_1",		cmd_1},
                            {"cmd_2",		cmd_2},
							{"read_humid", 	cmd_read_humid},
							{"read_temp",  	cmd_read_temp},
							{"read_th",    	cmd_read_th},
							{"set_cfg",    	cmd_set_cfg},
							{"get_cfg",    	cmd_get_cfg},
							{"read_xadc",	cmd_read_xadc},
							{""      ,   	NULL}};

char* cmd_process_str(char* input_str, char *output_str)
{
    int len;
	char *result_str;
	
	// check is command empty
    len = strlen(input_str);
    if (len<=0)
	{
		strcpy(output_str,"");
		return output_str;
	}

	/*	
	argv[0] = node name
	argv[1] = command
	argv[2] = 1st command parameter
	argv[3] = 2nd command parameter
	.
		.
	argc[n] = n-1 command parameter
	*/
	
	//check node name.
    argc=0;
    argv[argc] = strtok (input_str," ");
    //argv[argc]=NULL;
	if (strcmp(argv[0],"$$$") !=0) {
		if (strcmp(argv[0],node_name) !=0) {
			strcpy(output_str,"");
			return output_str;
		}
	}
	
	//add node name to output
	strcpy(output_str,node_name);
	len=strlen(output_str);
	output_str[len]=' ';
	result_str = output_str+len+1;
	
	while ((argv[argc] != NULL)&&(argc<MAX_ARG))
	{
		//printf ("%s\n",pch);
		argc++;
		argv[argc] = strtok (NULL, " ");
	}
	
	if (argc<=1) {
		strcpy(result_str,"i am alive");
		return output_str;
	}
		
	
	
    int i;
    i=0;
    while(strlen(cmd_table[i].cmd_str)>0)
    {
        if (strcmp(cmd_table[i].cmd_str,argv[1])==0) 
        {
			cmd_table[i].cmd_func(argc-2, argv+2, result_str);
            return output_str;
        }
        i++;
    }
	
    strcpy(output_str,"invalid command");
    return output_str;
    
}
  
  



  
  
  
  
	
	
