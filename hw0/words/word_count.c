/*

Copyright © 2019 University of California, Berkeley

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

word_count provides lists of words and associated count

Functional methods take the head of a list as first arg.
Mutators take a reference to a list as first arg.
*/

#include "word_count.h"

/* Basic utililties */

char *new_string(char *str) {
  return strcpy((char *)malloc(strlen(str)+1), str);
}

void init_words(WordCount **wclist) {
  /* Initialize word count.  */
  *wclist = NULL;
}

size_t len_words(WordCount *wchead) {
  if (wchead == NULL) {
    return 0;
  }
  return 1 + len_words(wchead->next);
}

WordCount *find_word(WordCount *wchead, char *word) {
  /* Return count for word, if it exists */
  WordCount *wc = wchead;
  while (wc != NULL) {
    if (strcmp(wc->word, word) == 0) {
      return wc;
    }
  }
  return NULL;
}


static bool wordcount_less(const WordCount *wc1, const WordCount *wc2) {
  if (wc1->count < wc2->count) {
    return 1;
  }
  else if (wc1->count > wc2->count) {
    return 0;
  } 
  else {
    // wcA = wcB in terms of count
    return strcmp(wc1->word, wc2->word);
  }
}

void add_word(WordCount **wclist, char *word) {
  /* If word is present in word_counts list, increment the count, otw insert with count 1. */
  WordCount* node = find_word(*wclist, word);
  if (node != NULL) {
    node->count = node->count + 1;
  }
  else {
    //wordcount_sort(WordCount **wclist, bool less(const WordCount *, const WordCount *))
    wordcount_sort(wclist, wordcount_less);

    // Create new WordCount object
    WordCount* countObject = (WordCount *) malloc(sizeof(WordCount));
    countObject->word = new_string(word);
    countObject->count = 1;
    countObject->next = *wclist;

    wordcount_sort(wclist, wordcount_less);
    // wordcount_insert_ordered(WordCount **wclist, WordCount *elem, bool less(const WordCount *, const WordCount *));
    // wordcount_insert_ordered(wclist, countObject, wordcount_less);
  }

}

void fprint_words(WordCount *wchead, FILE *ofile) {
  /* print word counts to a file */
  WordCount *wc;
  for (wc = wchead; wc; wc = wc->next) {
    fprintf(ofile, "%i\t%s\n", wc->count, wc->word);
  }
}
