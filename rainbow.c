#include <my_global.h>
#include <mysql.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>       /* time */
#include <inttypes.h>
#include <string.h>
#include "blake.h"

const int chainLength = 100000;
const int chainNumber = 200000;

void finish_with_error(MYSQL *con)
{
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  exit(1);        
}

int main(int argc, char **argv)
{  

  int i,j,cl,k;
  static const char alphanum[] =
        "0123456789!@"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
  uint8_t plaintext[7];
  uint8_t hash[33];
  char r[6];
  char statement[512];
  char temp[3];
  char hash64[65];

  srand(time(NULL));
  MYSQL *con = mysql_init(NULL);

  if (con == NULL) 
  {
      fprintf(stderr, "%s\n", mysql_error(con));
      exit(1);
  }

  if (mysql_real_connect(con, "localhost", "root", "botiotia", 
          "test", 0, NULL, 0) == NULL) 
  {
      fprintf(stderr, "%s\n", mysql_error(con));
      mysql_close(con);
      exit(1);
  }  

  //if (mysql_query(con, "CREATE DATABASE testdb")) 
  //{
      //fprintf(stderr, "%s\n", mysql_error(con));
      //mysql_close(con);
      //exit(1);
  //}

  if (mysql_query(con, "DROP TABLE IF EXISTS Rainbow")) 
  {
      finish_with_error(con);
  }

  if (mysql_query(con, "CREATE TABLE Rainbow(PlainText VARCHAR(7), Hash VARCHAR(65), PRIMARY KEY(Hash))")) 
  {      
      finish_with_error(con);
  }

  for(i = 0; i<chainNumber; i++)                     //num of chains
  {
  	for (j = 0; j < 6; ++j)               //random password
	{ 
        	plaintext[j] = alphanum[rand() % (sizeof(alphanum) - 1)];
    	}
    	plaintext[6] = '\0';

        blake256_hash(hash,plaintext,6);
        hash[64] = '\0';

    	for(cl=0;cl<chainLength;cl++)            //length of chain
    	{

    		Reduce(hash,cl,r);	
    		blake256_hash(hash,r,6);  

		for(k=0;k<32;k++)
	  	{	
	   		snprintf(temp,3,"%02x",hash[k]);
	   		hash64[2*k] = temp[0];
	   		hash64[2*k+1] = temp[1];
		}
	
                hash64[64] = '\0';
    	}    	
	hash[32] = '\0'; 

	for(k=0;k<32;k++)
	{	
	   snprintf(temp,3,"%02x",hash[k]);
	   hash64[2*k] = temp[0];
	   hash64[2*k+1] = temp[1];
	}
        hash64[64] = '\0';

	//printf("%s\n",hash);
        snprintf(statement, 512, "INSERT INTO Rainbow(PlainText,Hash) VALUES('%s','%s') ON DUPLICATE KEY UPDATE PlainText='%s'",plaintext,hash64,plaintext);
        if (mysql_query(con, statement) )
	{
      		finish_with_error(con);
  	}    
    	
    	//plaintext = "";
    	printf("End of chain: %d\n",i);
    }


  mysql_close(con);
  exit(0);
}


