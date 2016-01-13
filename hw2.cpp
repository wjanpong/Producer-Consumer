#include <iostream>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <values.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <semaphore.h>
#include <cstring>
#define _UNIX03_THREADS
#define _OPEN_THREADS

using namespace std;

#define NUM_THREADS 3

pthread_mutex_t M;//for fixed size buffer
pthread_mutex_t M1;//for tool1
pthread_mutex_t M2;//for tool2
pthread_mutex_t M3;//for tool3
pthread_mutex_t mutex_pause; //for pause and resume
pthread_mutexattr_t mattr;
pthread_cond_t  C;//for generator
pthread_condattr_t cattr;
pthread_cond_t  C1;//for operator
pthread_cond_t  C2;
pthread_cond_t cond_pause;
int op_num;
int tool_num;
int material1_count=0;
int material2_count=0;
int material3_count=0;
int dead_count=0;//deadlock count
int product1_count=0;
int product2_count=0;
int product3_count=0;

int isPaused = 0;


class queue
{
  public:

    queue(); // constructor - constructs a new empty queue.
    void enqueue( int item ); // enqueues <item>.
    int dequeue();  // dequeues the front item.
    int front();   // returns the front item without dequeuing it.
    bool empty();  // true iff the queue contains no items.
    int size();  // the current number of items in the queue.
    bool check( long pid); // show products
    bool recent( int product);//check for two same products cannot be next to each other
    bool check_entire(int product);//check the difference between any two kind should be less than 10
    void show();
    bool check_buffer(int product);
    int check_p();
    //int get_1st_tool(int tid);
    //int get_2nd_tool(int tool1,int tid);
    private:
    class node  // node type for the linked list
    {
       public:
           node(int new_data, node * next_node ){
              data = new_data ;
              next = next_node ;
           }
           int data ;
           node * next ;
    };

    node * front_p ; // pointer to the (node containing the) next item
              //  which will be dequeud, or NULL if the queue is empty.

    node * back_p ; // pointer to the (node containing the) last item
              // which was enqueued, or NULL if the queue is empty.

    int current_size ; // current number of elements in the queue.
};

queue fix_buffer;//input buffer with size10
queue output_Q;//output queue(unlimited)
queue::queue()
{
   front_p = NULL;
   back_p = NULL;
   current_size=0;
}

void queue::enqueue(int item)
{
   node *new_node= new node(item,NULL);
   if (front_p==NULL && back_p==NULL)//empty queue
   {
   back_p=new_node;
   front_p=new_node;
   }
   else
   {
   back_p->next=new_node;
   back_p=new_node;
   }
   current_size++;
}

int queue::dequeue()
{
   /*if (front_p==NULL && back_p==NULL)
   {
     cout<<"queue is alrady empty";
     break;
   }*/
   node * old_front = front_p;
   int temp=old_front->data;
   if( front_p==back_p)
   {
     front_p = NULL;
     back_p = NULL;
   }
   else
   {
   front_p=front_p->next;
   }
   delete old_front;
   current_size--;
   return temp;

}


int queue::size()
{
  return current_size;
}

bool queue::empty()
{
  return (front_p==NULL && back_p==NULL);
}

int queue::front()
{
   return front_p->data;
}

bool queue::check(long pid)
{
     node * temp = front_p;
     int track=0;
     for (int i=0; i<size();i++)
     {
        if (temp->data==pid)
        {
            track++;
        }
        temp=temp->next;
    }
    if (track>=4)
    {
    return false;
    }
    else
    {
    return true;
    }
}

bool queue::recent(int product)
{
    if (back_p->data==product)
    {
       return false;
    }

    return true;
}

bool queue::check_buffer(int material)
{

    int count1=0;
    int count2=0;
    int count3=0;
    node * temp = front_p;
    for (int i=0; i<size();i++)
    {
       if (temp->data==1)
       {
         count1++;
       }
       else if (temp->data == 2)
       {
         count2++;
       }
       else
       {
          count3++;
       }
       temp=temp->next;
    }
    if (((count1-count2>3)||(count1-count3)>3) && (material==1))
    {
        return false;
    }
    else if (((count2-count1>3)||(count2-count3)>3) && (material==2))
    {
        return false;
    }
    else if (((count3-count2>3)||(count3-count1)>3) && (material==3))
    {
        return false;
    }
    else
    {
        return true;
    }
}


bool queue::check_entire(int product)
{
    int count1=0;
    int count2=0;
    int count3=0;
    node * temp = front_p;
    for (int i=0; i<size();i++)
    {
       if (temp->data==3)
       {
         count1++;
       }
       else if (temp->data == 4)
       {
         count2++;
       }
       else
       {
          count3++;
       }
       temp=temp->next;
    }
    if (((count1-count2>10)||(count1-count3)>10) && (product==3))
    {
        return false;
    }
    else if (((count2-count1>10)||(count2-count3)>10) && (product==4))
    {
        return false;
    }
    else if (((count3-count2>10)||(count3-count1)>10) && (product==5))
    {
        return false;
    }
    else
    {
        return true;
    }
}

void queue::show()
{
    node *temp=front_p;
    cout<<"show: "<<endl;
    for (int i=0; i<this->size();i++)
    {

       cout<<temp->data<<endl;
       temp=temp->next;
    }
}

int queue::check_p()
{
   node *t=front_p;
   int first = t->data;
   t=t->next;
   int second = t->data;
   int check_product = first+second;
   return check_product;
}

/*int queue::get_1st_tool(int tid)
{

   int random_tool1=rand() % 3 + 1;
   if (random_tool1==1)
   {
        if(pthread_mutex_trylock(&M1)==0)
        {
                //tools.enqueue(1);
                cout<<"get tools 1 by operator "<<tid<<endl;
                //tools.show();
                return 1;
        }

    }
    else if (random_tool1==2)
    {
        if(pthread_mutex_trylock(&M2)==0)
        {
                //tools.enqueue(2);
                cout<<"get tools 2 by operator "<<tid<<endl;
                //tools.show();
                return 2;
        }

    }
    else if (random_tool1==3)
    {
        if(pthread_mutex_trylock(&M3)==0)
        {
               //tools.enqueue(3);
                cout<<"get tools 3 by operator "<<tid<<endl;
                //tools.show();
                return 3;
        }

    }
}*/



/*int queue::get_2nd_tool(int tool1, int tid)
{
    int flag=true;
    while (flag)
    {
            int random_tool2=rand() % 3 + 1;
            if (random_tool2!=tool1)
            {
                if (random_tool2==1)
                {
                    if(pthread_mutex_trylock(&M1)==0)
                    {
                        //tools.enqueue(1);
                        cout<<"get tools 1 by operator "<<tid<<endl;
                        //tools.show();
                        flag=false;
                        return 1;
                    }

                }
                else if (random_tool2==2)
                {
                    if(pthread_mutex_trylock(&M2)==0)
                    {
                        //tools.enqueue(2);
                        cout<<"get tools 2 by operator "<<tid<<endl;
                        //tools.show();
                        flag=false;
                        return 2;
                    }

                }
                else if (random_tool2==3)
                {
                    if(pthread_mutex_trylock(&M3)==0)
                    {
                        //tools.enqueue(3);
                        cout<<"get tools 3 by operator "<<tid<<endl;
                        //tools.show();
                        flag=false;
                        return 3;
                    }

                }
                }
    }
}*/


void * generators(void *threadid)
{
   long tid;
   tid = (long)threadid;

   printf("Hello World! It's me, generator_thread #%ld!\n", tid);
   while(true)//for (int i=0; i<10; i++)//change to while(true)
   {
        pthread_mutex_lock(&mutex_pause);
        while (isPaused == 1)
        {
            pthread_cond_wait(&cond_pause, &mutex_pause);
        }
        //else
        //{
            pthread_mutex_unlock(&mutex_pause);
        //int rand_time=rand() % 10000;
        //usleep(rand_time);
        pthread_mutex_lock(&M);//lock mutex


        //critical section
        int material=tid;
        if ((!fix_buffer.check(tid)) || (!fix_buffer.check_buffer(material)) || (fix_buffer.size() >=10) )
        {
            dead_count++;
            cout<<"Deadlock due to full or too many of the material in the buffer!!!"<<endl;
            cout<<"Number of deadlock happened is "<<dead_count<<endl;
        }
        while ((!fix_buffer.check(tid)) || (!fix_buffer.check_buffer(material)) || (fix_buffer.size() >=10) )//check if there are many of those products
        if (pthread_cond_wait (&C, &M))//producer of the material goes to wait
        {
          fprintf (stdout, "pthread_cond_wait: producer\n");
          exit (-1);
        }
        /*int material=tid;
        while(!fix_buffer.check_buffer(material))// wait if the difference are 10
        if (pthread_cond_wait (&C, &M))
        {
            fprintf (stdout, "pthread_cond_wait: consumer\n");
            exit (-1);
        }*/

        printf(" thread #%ld produces %ld\n", tid, material);

        if (fix_buffer.size() >= 10)
        {
            cout<<"Full Wait"<<endl;

        }
        /*while (fix_buffer.size() >=10)//if its full wait
        if (pthread_cond_wait ( &C1 , &M))
        {
          fprintf (stdout, "pthread_cond_wait: producer\n");
          exit (-1);
        }*/
        fix_buffer.enqueue(material);
        //pthread_cond_signal (&C1);
        if (material==1)
        material1_count++;
        else if (material==2)
        material2_count++;
        else
        material3_count++;
        cout<<"current size of fix_buffer is "<<fix_buffer.size()<<endl;
        fix_buffer.show();
        cout<<material1_count<<" of material 1 are generated\n";
        cout<<material2_count<<" of material 2 are generated\n";
        cout<<material3_count<<" of material 3 are generated\n";
        pthread_mutex_unlock (&M);

        // noncritical section
        pthread_cond_signal (&C1);
    }
    //}
    pthread_exit(NULL);
}


void *operators(void *threadid)
{
   long tid;
   tid = (long)threadid;
   printf("Hello World! It's me, operator_thread #%ld!\n", tid);
   queue require;
   queue tools;



   while(true)//for (int i=0; i<10; i++)//change to while(true)
   {


        pthread_mutex_lock(&mutex_pause);
        while (isPaused == 1)
        {
            pthread_cond_wait(&cond_pause, &mutex_pause);
        }
        //else
        //{
        pthread_mutex_unlock(&mutex_pause);
        int rand_time=rand() % 10000;
        usleep(rand_time);



        pthread_mutex_lock(&M);//lock mutex
        //critical section

        if (fix_buffer.size()==0)
        {
            cout<<"Empty"<<endl;
            dead_count++;
            cout<<"Deadlock due to Empty!!!"<<endl;
            cout<<"Number of deadlock happened is "<<dead_count<<endl;
        }
        while(fix_buffer.size()==0)//wait if fixed size buffer is 0
        if (pthread_cond_wait (&C1, &M))
        {
            fprintf (stdout, "pthread_cond_wait: consumer\n");
            exit (-1);
        }

        if (require.size()==0)//get 1st material
        {
            int temp=fix_buffer.dequeue();
            pthread_cond_signal (&C);
            require.enqueue(temp);
            cout<<"get "<<temp<<"by "<<tid<<endl;
            cout<<"require: "<<endl;
            require.show();
        }

        else if (require.size()==1)//get 2nd material
        {

             int temp1=fix_buffer.dequeue();
             pthread_cond_signal(&C);
             if (temp1 != require.front())
             {
                    require.enqueue(temp1);
                    cout<<"get "<<temp1<<"by "<<tid<<endl;
                    cout<<"require: "<<endl;
                    require.show();
                    int check_product=require.check_p();

                    if (output_Q.size()==0) //starting queue no need to check for previous product
                    {
                           int random_tool1;
                           int random_tool2;
                           if(pthread_mutex_trylock(&M1)==0)
                            {
                                       while(tools.size()<2)
                                       {
                                            if (tools.size()==0)
                                            {

                                                random_tool1=rand() % tool_num + 1;
                                                tools.enqueue(random_tool1);
                                                cout<<"get tools"<<random_tool1<<" by operator "<<tid<<endl;
                                                tools.show();
                                            }
                                            else if (tools.size()==1)
                                            {
                                                random_tool1=rand() % tool_num + 1;
                                                if (!(random_tool1==random_tool2))
                                                {
                                                   tools.enqueue(random_tool1);
                                                    cout<<"get tools"<<random_tool1<<" by operator "<<tid<<endl;
                                                    tools.show();
                                                }
                                            }
                                    }
                            }
                            int product=require.dequeue();
                            product=product+require.dequeue();
                            output_Q.enqueue(product);
                            cout<<"output_Q: "<<endl;
                            output_Q.show();
                            if (product==3)
                            product1_count++;
                            else if (product==4)
                            product2_count++;
                            else
                            product3_count++;
                            cout<<product1_count<<" of product 3 are generated\n";
                            cout<<product2_count<<" of product 4 are generated\n";
                            cout<<product3_count<<" of product 5 are generated\n";
                        }
                        else// if it is not 1st product
                        {
                            if ((!output_Q.recent(check_product)) || (!output_Q.check_entire(check_product)))
                            {

                                    dead_count++;
                                    cout<<"Deadlock due to recent product is same as new product or different number of products are more than 10 !!!"<<endl;
                                    cout<<"Number of deadlock happened is "<<dead_count<<endl;
                                    while(fix_buffer.size() >=10) //check if there are many of those products
                                    if (pthread_cond_wait (&C1, &M))//producer of the material goes to wait
                                    {
                                        fprintf (stdout, "pthread_cond_wait: producer\n");
                                        exit (-1);
                                    }
                                    //for (int i=0;i<2;i++)
                                    //{

                                    int put_back=require.dequeue();
                                    fix_buffer.enqueue(put_back);
                                    pthread_cond_signal (&C1);
                                    cout<<"Prevent Deadlock put : "<<put_back<<" to buffer"<<endl;
                                    fix_buffer.show();
                                    cout<<"current size of fix_buffer is "<<fix_buffer.size()<<endl;
                                    //}
                            }
                                    /*while(!output_Q.recent(product))//if recent is of same kind then wait
                                    if (pthread_cond_wait (&C2, &M))
                                    {
                                        fprintf (stdout, "pthread_cond_wait: consumer\n");
                                        exit (-1);
                                    }
                                    while(!output_Q.check_entire(product))// wait if the difference are 10
                                    if (pthread_cond_wait (&C2, &M))
                                    {
                                        fprintf (stdout, "pthread_cond_wait: consumer\n");
                                        exit (-1);
                                    }*/

                            else
                            {
                                int random_tool1;
                                int random_tool2;
                                    if(pthread_mutex_trylock(&M1)==0)
                                    {
                                       while(tools.size()<2)
                                       {
                                            if (tools.size()==0)
                                            {

                                                random_tool1=rand() % tool_num + 1;
                                                tools.enqueue(random_tool1);
                                                cout<<"get tools"<<random_tool1<<" by operator "<<tid<<endl;
                                                tools.show();
                                            }
                                            else if (tools.size()==1)
                                            {
                                                random_tool1=rand() % tool_num + 1;
                                                if (!(random_tool1==random_tool2))
                                                {
                                                   tools.enqueue(random_tool1);
                                                    cout<<"get tools"<<random_tool1<<" by operator "<<tid<<endl;
                                                    tools.show();
                                                }
                                            }
                                    }
                                    int product=require.dequeue();
                                    product=product+require.dequeue();
                                    cout<<"product is "<<product<<"  by tools"<<tools.dequeue()<<" and "<<tools.dequeue()<<endl;
                                    output_Q.enqueue(product);
                                    cout<<"output_Q: "<<endl;
                                    output_Q.show();
                                    cout<<"current size of output queue is "<<output_Q.size()<<endl;
                                    if (product==3)
                                    product1_count++;
                                    else if (product==4)
                                    product2_count++;
                                    else
                                    product3_count++;
                                    cout<<product1_count<<" of product 3 are generated\n";
                                    cout<<product2_count<<" of product 4 are generated\n";
                                    cout<<product3_count<<" of product 5 are generated\n";
                            }
                            }
                        }//end of not 1st output


                }// end of if (temp1 ! = require.front())


                else if ((temp1 == require.front()))
                {
                     dead_count++;
                     cout<<"Deadlock due to both materials are same!!!"<<endl;
                     cout<<"Number of deadlock happened is "<<dead_count<<endl;
                     fix_buffer.enqueue(temp1);
                     pthread_cond_signal (&C1);
                }
               }
                pthread_mutex_unlock ( &M);
                pthread_mutex_unlock ( &M1);
                pthread_mutex_unlock ( &M2);
                pthread_mutex_unlock ( &M3);
                //non critical section
                pthread_cond_signal (&C);
              //}//if pause
             }
                pthread_exit(NULL);


}

















int main (int argc, char *argv[])
{
   pthread_mutex_init(&M, NULL);
   pthread_mutex_init(&M1, NULL);
   pthread_mutex_init (&M2, NULL);
   pthread_mutex_init (&M3, NULL);
   pthread_mutex_init (&mutex_pause, NULL);
   pthread_cond_init (&C, NULL);
   pthread_cond_init (&C1, NULL);
   pthread_cond_init (&C2, NULL);
   pthread_cond_init (&cond_pause, NULL);
   pthread_t producer[NUM_THREADS];
   cout<<"Enter number of operators"<<endl;
   cin>>op_num;
   cout<<"Enter number of tools"<<endl;
   cin>>tool_num;
   pthread_t consumer[op_num];
   int result;
   long t;

//creating generators threads
   for(t=1; t<=NUM_THREADS; t++)
   {
      printf("In main: creating generators_thread %ld\n", t);
      result = pthread_create(&producer[t], NULL, generators, (void *)t);
      if (result)
      {
         printf("ERROR; return code from pthread_create() is %d\n", result);
         exit(-1);
      }
   }

//creating operators threads
   for(t=1; t<=op_num; t++)
   {
      printf("In main: creating operators_thread %ld\n", t);
      result = pthread_create(&consumer[t], NULL, operators, (void *)t);
      if (result)
      {
         printf("ERROR; return code from pthread_create() is %d\n", result);
         exit(-1);
      }
   }

   while(true)
   {
      //press p and enter to pause
      // press r and enter to resume
      // press q and enter to quit the program
      char ch;
      //cin.get(ch);
      if(cin.get(ch))
      {
          //char ch;
          //cin.get(ch);
          if (ch=='p')   // press p and enter to pause
          {
          pthread_mutex_lock(&mutex_pause);
          isPaused = 1;
          pthread_mutex_unlock(&mutex_pause);
          }
          else if (ch=='r')//(cin.get()=='r')//press r and enter to resume
          {
          pthread_mutex_lock(&mutex_pause);
          isPaused = 0;
          pthread_mutex_unlock(&mutex_pause);
          pthread_cond_signal (&cond_pause);
          pthread_cond_signal (&cond_pause);
          pthread_cond_signal (&cond_pause);
          pthread_cond_signal (&cond_pause);
          pthread_cond_signal (&cond_pause);
          pthread_cond_signal (&cond_pause);
          //pthread_mutex_unlock(&mutex_pause);
          }
          else if (ch == 'q')//(cin.get()=='q')//press q and enter to quit
          {
             pthread_mutex_lock(&mutex_pause);
             return false;
          }
       }

    }

   pthread_exit(NULL);
}


