/*
 * Word count application with one thread per input file.
 *
 * You may modify this file in any way you like, and are expected to modify it.
 * Your solution must read each input file from a separate thread. We encourage
 * you to make as few changes as necessary.
 */

/*
 * Copyright © 2019 University of California, Berkeley
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <pthread.h>

#include "word_count.h"
#include "word_helpers.h"

struct Data {
  char * string;
  word_count_list_t word_counts;
};





/*
 * main - handle command line, spawning one thread per file.
 */
int main(int argc, char *argv[]) {

  


  /* Create the empty data structure. */
  word_count_list_t word_counts;
  init_words(&word_counts);
  //printf("hello ever");

  void *threadfun(void *threadid) {

      char * dataFileName = (char * ) threadid; 
      // struct Data * datastruct = (struct Data *) threadid;  
      printf("%s\n", dataFileName);

      if (strcmp(dataFileName, "stdin") == 0) {
        count_words(&word_counts, stdin);
      }
      else {
        FILE *infile = fopen(dataFileName, "r");
        //word_count_list_t word_counts = word_counts;
        count_words(&word_counts, infile);
        fclose(infile);
      }

      pthread_exit(NULL);
  }


  pthread_t threads[argc];

  if (argc <= 1) {
    /* Process stdin in a single thread. */
    pthread_create(&threads[0], NULL, threadfun, (void *) "stdin");
    // count_words(&word_counts, stdin);
    pthread_join(threads[0], NULL);
  } else {
    /* TODO */
    int i;
    // struct Data datastruct;

    for (i = 1; i < argc; i++) {

      //datastruct.string = strcpy(malloc(100), argv[i]);
      //printf("%s", datastruct.string);

      // data.infile = fopen(argv[i], "r");
      //datastruct.word_counts = word_counts;

      int rc = pthread_create(&threads[i - 1], NULL, threadfun, (void *) argv[i]);


      if (rc) {
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
      }

    }

    for (int j = 1; j < argc; j++) {
      pthread_join(threads[j - 1], NULL);
    }


  }

  /* Output final result of all threads' work. */
  wordcount_sort(&word_counts, less_count);
  fprint_words(&word_counts, stdout);

  pthread_exit(NULL);
}
