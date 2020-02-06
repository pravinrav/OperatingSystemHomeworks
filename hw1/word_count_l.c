/*
 * Implementation of the word_count interface using Pintos lists.
 *
 * You may modify this file, and are expected to modify it.
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

#ifndef PINTOS_LIST
#error "PINTOS_LIST must be #define'd when compiling word_count_l.c"
#endif

#include "word_count.h"

char *new_string(char *str) {
  return strcpy((char *)malloc(strlen(str)+1), str);
}

void init_words(word_count_list_t *wclist) {
  /* TODO */
  // wclist is a pointer to a word_count_list_t object - pointer to a struct list
  // word_count_list_t is a struct list

  list_init(wclist);

  // struct list birthdate_list;
  // list_init (&birthdate_list);
}

size_t len_words(word_count_list_t *wclist) {
  /* TODO */
  return list_size(wclist);
}

word_count_t *find_word(word_count_list_t *wclist, char *word) {
  /* TODO */
  struct list_elem *e;
  for (e = list_begin(wclist); e != list_end(wclist); e = list_next(e)) {

    // word_count_t * wc = (word_count_t *) malloc(sizeof(word_count_t));
    word_count_t * wc = list_entry(e, word_count_t, elem);

    if (strcmp(wc->word, word) == 0) {
      return wc; 
    }
  }
  return NULL;
}

word_count_t *add_word_with_count(word_count_list_t *wclist, char *word,
                                  int count) {
  /* TODO */
  word_count_t * node = find_word(wclist, word);
  if (node != NULL) {
    node->count = node->count + 1;
    return node;
  } else {

    // Allocate space for the new word
    word_count_t * newNode = (word_count_t *) malloc(sizeof(word_count_t));

    newNode->word = new_string(word);
    newNode->count = 1;

    list_push_front(wclist, &newNode->elem);

    return newNode;
  }
}
// blah blah
word_count_t *add_word(word_count_list_t *wclist, char *word) {
  return add_word_with_count(wclist, word, 1);
}

void fprint_words(word_count_list_t *wclist, FILE *outfile) {
  struct list_elem *e;
  for (e = list_begin(wclist); e != list_end(wclist); e = list_next(e)) {
    word_count_t * wc = list_entry(e, word_count_t, elem);
    fprintf(outfile, "%8d\t%s\n", wc->count, wc->word);
  }
}

static bool less_list(const struct list_elem *ewc1,
                      const struct list_elem *ewc2, void *aux) {
  /* TODO */
  bool (*functPtr)(word_count_t *, word_count_t *) = aux;

  // void (*fun_ptr)(int) = aux; 

  return (*functPtr)(list_entry(ewc1, word_count_t, elem), list_entry(ewc2, word_count_t, elem));
}

void wordcount_sort(word_count_list_t *wclist,
                    bool less(const word_count_t *, const word_count_t *)) {
  list_sort(wclist, less_list, less);
}
