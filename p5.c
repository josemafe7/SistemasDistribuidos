/*
EPD08-P5

Escriba un programa que calcule la multiplicación de dos matrices (si fuera necesario, puede consultar Internet para
refrescar este concepto). Dicho programa tendrá que ser codificado de forma distribuida (con reparto de carga entre
procesos con MPI) y concurrente (ejecución multihilo de dichas cargas con OpenMP). 

*/

/*
Autor: José_Manuel_Fernández_Labrador
*/

#include <mpi.h>
#include <stdio.h>
#include <omp.h>

#define NRA 4                 
#define NCA 3                
#define NCB 5                  
#define MASTERPID 0              
#define TAG 0

int main (int argc, char *argv[])
{  
  
	int pid=0, prn=0, spid=0,numesclavos=0,rows=0,rowdiv=0,rowrest=0, offset=0, i=0, j=0, k=0, rc=0;
   	double	a[NRA][NCA], b[NCA][NCB], c[NRA][NCB];           

   	MPI_Init(&argc,&argv);
   	MPI_Comm_rank(MPI_COMM_WORLD,&pid);
   	MPI_Comm_size(MPI_COMM_WORLD,&prn);

   	numesclavos = prn-1;
   	rowdiv  = NRA/numesclavos;
   	rowrest = NRA%numesclavos;

   	if (pid==MASTERPID) {
     
      		//** Inicialización de matrices
      		for (i=0; i<NRA; i++)
         		for (j=0; j<NCA; j++)
            			a[i][j]= i+j;
      
		for (i=0; i<NCA; i++)
         		for (j=0; j<NCB; j++)
            		b[i][j]= i*j;


      		offset = 0;
      		//** Envío a los esclavos: número de filas a procesar, parte de la matriz A y matriz B completa a los esclavos
      		for (spid=1; spid<=numesclavos; spid++){
         
         		rows = (spid <= rowrest) ? rowdiv+1 : rowdiv;   

         		// printf("[%d] --> [%d] rows = %d, offset = %d\n",pid,spid,rows,offset);
         		MPI_Send(&offset, 1, MPI_INT, spid, TAG, MPI_COMM_WORLD);
         		MPI_Send(&rows, 1, MPI_INT, spid, TAG, MPI_COMM_WORLD);
         		MPI_Send(&a[offset][0], rows*NCA, MPI_DOUBLE, spid, TAG,MPI_COMM_WORLD);
         		MPI_Send(&b, NCA*NCB, MPI_DOUBLE, spid, TAG, MPI_COMM_WORLD);
         		offset = offset + rows;
      		}

      		//** Recepción de resultados
      		for (spid=1; spid<=numesclavos; spid++) {
         		MPI_Recv(&offset, 1, MPI_INT, spid, TAG, MPI_COMM_WORLD, NULL);
         		MPI_Recv(&rows, 1, MPI_INT, spid, TAG, MPI_COMM_WORLD, NULL);
         		MPI_Recv(&c[offset][0], rows*NCB, MPI_DOUBLE, spid, TAG, MPI_COMM_WORLD, NULL);
     		}

     		//** Impresión
       		printf("Matriz resultante:\n");
      		for (i=0; i<NRA; i++) {
         		printf("\n"); 
         			for (j=0; j<NCB; j++) 
            				printf("%5.2f   ", c[i][j]);
      		}
      		printf("\n"); 
   	}

   	else{
      
      		MPI_Recv(&offset, 1, MPI_INT, MASTERPID, TAG, MPI_COMM_WORLD, NULL);
      		MPI_Recv(&rows, 1, MPI_INT, MASTERPID, TAG, MPI_COMM_WORLD, NULL);
      		MPI_Recv(&a, rows*NCA, MPI_DOUBLE, MASTERPID, TAG, MPI_COMM_WORLD, NULL);
      		MPI_Recv(&b, NCA*NCB, MPI_DOUBLE, MASTERPID, TAG, MPI_COMM_WORLD, NULL);

      		#pragma omp parallel for
      		for (k=0; k<NCB; k++)
         		for (i=0; i<rows; i++){
            			c[i][k] = 0.0;
            			for (j=0; j<NCA; j++)
               				c[i][k] = c[i][k] + a[i][j] * b[j][k];
         		}
      
      		MPI_Send(&offset, 1, MPI_INT, MASTERPID, TAG, MPI_COMM_WORLD);
      		MPI_Send(&rows, 1, MPI_INT, MASTERPID, TAG, MPI_COMM_WORLD);
      		MPI_Send(&c, rows*NCB, MPI_DOUBLE, MASTERPID, TAG, MPI_COMM_WORLD);
   	}

   	MPI_Finalize();
}


