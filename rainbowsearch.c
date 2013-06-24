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

void main()
{
  MYSQL *con = mysql_init(NULL);
  char statement[512];
  char myHash[] =        "d5852d4dac50bbc61d6cb794f99bdf0f12cc5220dd2d043bbde18ebedd3ca95f";
  unsigned char dynamicHash[32];
  char *plaintext = NULL;
  char *currentHash = NULL;
  char hash64[65];
  char temp[3];
  char r[6];
  int num_fields,i,k;
  MYSQL_ROW row;
  MYSQL_RES *result;
  int successFlag = 0;
  
  char *pos = myHash;
  size_t count = 0;    
     /* Convert back from hex string to byte array */
  for(count = 0; count < sizeof(dynamicHash)/sizeof(dynamicHash[0]); count++) 
  {
      sscanf(pos, "%2hhx", &dynamicHash[count]);
      pos += 2 * sizeof(char);
  }


  plaintext = malloc(7);
  currentHash = malloc(33);   

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

   for(k=0;k<32;k++)
   {	
	snprintf(temp,3,"%02x",dynamicHash[k]);
	hash64[2*k] = temp[0];
	hash64[2*k+1] = temp[1];
   }
   hash64[64] = '\0';

   for(i = chainLength-1; i >= 0; i--)      //iterate chain backwards
   {
	snprintf(statement, 512, "SELECT * FROM Rainbow where Hash = '%s'",hash64);
  	if (mysql_query(con, statement) )
  	{
  		finish_with_error(con);
  	}
	result = mysql_store_result(con); 
        if (result == NULL) 
  	{
      		finish_with_error(con);
 	}
  	row = mysql_fetch_row(result);  //we know that if any result is produced, it is going to be distinct from creation and thus no iteration with num_fields is required.(maximum 1 row per query)

        if(row != NULL)              //if found a result
	{
           
	   if(plaintext == NULL)
	       plaintext = malloc(7);
	   if(currentHash == NULL)  
	       currentHash = malloc(33); 
           //copy to the corresponding buffers
	   strncpy(plaintext,row[0],7);                         //row[0] to plaintext        
           char *pos1 = row[1];					//now convert this row[1] to byte array and copy it to hash
           size_t count = 0;    
           /* Convert back from hex string to byte array, because the R and Blake take the byte array as a parameter */
           for(count = 0; count < sizeof(currentHash)/sizeof(currentHash[0]); count++) 
           {
              sscanf(pos1, "%2hhx", &currentHash[count]);
              pos1 += 2 * sizeof(char);
           }
	}
	else                             //else both NULL
	{
	   plaintext = NULL;
	   currentHash = NULL;
        }
	
	if(currentHash != NULL)         //hash is found.
	{   
	    blake256_hash(dynamicHash,plaintext,6);         //start from the beginning of the chain
	    for(k = 0; k <= i; k++)         //iterate through all the R functions until the hash
	    {
		Reduce(dynamicHash,k,r);           //k is the position
                //printf("%s\n",currentHash);
	  	if(i == k)
		{
			printf("Found at %d. The password is: %s \n",k,r);
			successFlag = 1;
		}
		blake256_hash(dynamicHash,r,6);
	    }
            i = -1;                     //to stop the iteration
	}
        Reduce(dynamicHash,i,r);                
        blake256_hash(dynamicHash,r,6);          
        
	for(k=0;k<32;k++)
	{	
	   snprintf(temp,3,"%02x",dynamicHash[k]);
	   hash64[2*k] = temp[0];
	   hash64[2*k+1] = temp[1];
	}
        hash64[64] = '\0';
        if((i % 10000) == 0)
           printf("%d steps from %d\n",chainLength-i,chainLength);
   }

  if(!successFlag)
      printf("Password not Found :-(\n");

  mysql_free_result(result);
  mysql_close(con);
  
  exit(0);

}
