#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
typedef struct{ //struct holding variables I need to pass  to pthread_create, so they will be passed as a pointer to this struct
    int i;
    int k;
    int n;
    int o;
}compute_arg;
//Global arrays ...
int a[1000][10000];
int b[1000][10000];
int c[1000][10000];


void read_matrices(int*m,int*n,int*o);//pass integer ppointer to store dimensions of matrices
void compute_element(compute_arg*);
double multiply(int,int,int); //returns the time elapsed to multiply
double multiplybyrow(int,int,int); //returns the time elapsed to multiply
void compute_row(compute_arg*);
void write_output(int,int,double,int);
int main()
{

    int m,n,o;
    read_matrices(&m,&n,&o); //read matrices and the dimennsions and put them into n,m,o

    double t1=multiply(m,n,o);
    //call the function to multiply where each thread computes a single element in the product matrix
    //returns time taken by the procedure
    write_output(m,o,t1,0);//mode = 0 to write product of first procedure

    double t2=multiplybyrow(m,n,o);//multiply matrix,, each thread computes a single row in the product matrix,, returns time taken

    write_output(m,o,t2,1);//mode =1 to write produc 2nd procedure
    /*for(int i=0;i<m;i++){
        for(int j=0;j<o;j++){
            printf("%d ",c[i][j]);
        }
        printf("\n");
    }*/
    return 0;
}
void read_matrices(int*m1,int*n1,int*o1){ // passing  pointers to the variables holding the dimensions
    FILE* fptr =fopen("input.txt","r");
   // printf("succes");
    fscanf(fptr,"%d %d\n",m1,n1);  // no need for ampersand,, they are already pointers.
    int m=*m1; int n=*n1;

    for(int i=0;i<m;i++){
        for(int j=0;j<n;j++){
            fscanf(fptr,"%d",&a[i][j]);
        }
        fscanf(fptr,"\n");//skip new line  ; go the next row
    }

    fscanf(fptr,"%d %d\n",n1,o1);//read n again and read o
    int o=*o1;
    for(int i=0;i<n;i++){
        for(int j=0;j<o;j++){
            fscanf(fptr,"%d",&b[i][j]);
        }
        fscanf(fptr,"\n");//skip new line : go to the next row
    }
    fclose(fptr);
    free(fptr);

}
void compute_element(compute_arg *args){

    int i,k;
    int n;
    n=args->n;
    i=args->i;
    k=args->k;
    // copy arguments to the local variables to avoid using arrows alot
    free(args);
    //printf("--%d-%d-]",i,k);
    c[i][k]=0;// clear it before operating because you will keep adding to it
    //int c=-1;
    for(int j=n-1;j>=0;j--){
        c[i][k]+= a[i][j] *b[j][k];
    }
    //printf("%d ",c[i][k]);
    //return -1; // I dont need it to return anything actually

}

double multiply(int m,int n,int o){ //return the time

    /*int **c=malloc(m*sizeof(int *));//the product matrix is m x o
    for(int i=0;i<m;i++){
        c[i]=malloc(n*sizeof(int));
    }*/

    pthread_t threads[m*o]; //you need m*o threads for m*o elements;
    int thread_idx=0;
    clock_t start_time,finish_time;

    start_time = clock();
    for(int k=0;k<o;k++)
        for(int i=0;i<m;i++){

            compute_arg* arg=malloc(sizeof(compute_arg));//allocate memory for the arguments passed to compute_element,, it will be freed inside the compute_element
            //maybe it's not good to malloc and then free could take time
            arg->i=i;
            arg->k=k;
            //index of current element is [i][k] so pass it the function
            arg->n=n; // pass it because it is the number of columns in the 1st matrix,and number of rows in the 2nd matrix
            arg->o=0;//do not need here,, but I am afraid of garbage
            //pass the needed arguments through the pointer
            pthread_create(&threads[thread_idx++],0,compute_element,arg);


        }

    for(int i=0;i<thread_idx;i++){ //wait for all the threads to finish their work before returning

        pthread_join(threads[i],0); //0 is the same as NULL;
    }
    finish_time = clock();

    double time_elapsed =((double)(finish_time-start_time) )/CLOCKS_PER_SEC  ;// time
    return time_elapsed;
}
double multiplybyrow(int m,int n,int o){  //return the time

    //overrite c array no need for extra space since this function will be called after writing the result of the other function.

    pthread_t threads[m];// m threads for m rows
    clock_t start_time,finish_time;

    start_time = clock();// start time = current time

    for(int i=0;i<m;i++) //fill collumns first
    {
        compute_arg *arg=malloc(sizeof(compute_arg));
        arg->i=i; //pass the index of current row in resultant matrix C
        arg->o=o;//pass number of columns in the resultant array
        arg->n=n; //pass number of columns in the  1st matrix ,, and number of rows in the 2nd matrix
        arg->k=0;//dont need

        pthread_create(&threads[i],0,compute_row,arg);
    }
    for(int i=0;i<m;i++){ // wait for all rows to finish
        pthread_join(threads[i],0);
    }

    finish_time = clock()-start_time;// current time
    double time_elapsed =((double)(finish_time))/CLOCKS_PER_SEC;// get time in ms

    //printf("time%lftine",time_elapsed);


    return time_elapsed; //should return time

}
void compute_row(compute_arg* arg){

    int i = arg->i;
    int o = arg->o;
    int n = arg->n;
    //create local variable copies of the arguments so the code can be readable.
    free(arg);

    for(int k=0;k<o;k++){
        c[i][k]=0;
        for(int j=n-1;j>=0;j--){
            c[i][k]+=a[i][j]*b[j][k];
        }

    }


}
void write_output(int m,int o,double time,int mode){  // mode = 0 :write result of thread/element,, mode=1 write result of thread/row
    if(mode!=0 && mode!=1){ // I don't know who will call it with anything other than 0 or 1 , but just in case
        return;
    }
    FILE* fptr=NULL;
    switch(mode){

        case 0:{  // multiply thread per element so create a new file
            fptr=fopen("output.txt","w");

            break;
        }
        case 1:{ //multipy thread per row,, append to the file created from the previous call

            fptr =fopen("output.txt","a+");
            break;
        }
    }
    for(int i=0;i<m;i++){
        for(int j=0;j<o;j++){
            fprintf(fptr,"%d ",c[i][j]);
        }
        fprintf(fptr,"\n");
    }
    fprintf(fptr,"END%d    [%.09lf]\n",mode+1,time); //write the time in the output file ,, need good precision so. 09lf
    fclose(fptr);
    free(fptr);

}
