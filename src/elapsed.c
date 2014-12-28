#include <stdio.h> 
#include <unistd.h> 
#include <time.h> 
#include <sys/times.h> 
#include <sys/wait.h> 

int main(int argc, char *argv[]) { 

   struct tms InfoInicio, InfoFin; 
   clock_t t_inicio, t_fin; 
   long tickporseg; 

   if (argc<2) 
   { 
      fprintf(stderr, "Uso: %s programa [args]\n", argv[0]); 
   } 
   else
   {  
      /* obtiene el número de int. De reloj por segundo */ 
      tickporseg= sysconf(_SC_CLK_TCK); 
      t_inicio= times(&InfoInicio); 
   
      if (fork()==0) 
      { 
         execvp(argv[1], &argv[1]); 
         perror("error ejecutando el programa"); 
   
      }
      else
      {
    
         wait(NULL); 
   
         t_fin= times(&InfoFin); 
   
         printf ("Tiempo real: %7.2f\n", (float)(t_fin - t_inicio)/tickporseg); 
         printf ("Tiempo de usuario: %7.2f\n", 
            (float)(InfoFin.tms_cutime - InfoInicio.tms_cutime)/tickporseg); 
         printf ("Tiempo de sistema: %7.2f\n", 
            (float)(InfoFin.tms_cstime - InfoInicio.tms_cstime)/tickporseg); 

      }
   }
   return 0; 
   
}

