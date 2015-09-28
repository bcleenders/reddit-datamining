//  Copyright 2013 Google Inc. All Rights Reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "malloc.h"
#include <stdlib.h>

const long long max_size = 2000;         // max length of strings
const long long N = 40;                  // number of closest words that will be shown
const long long max_w = 50;              // max length of vocabulary entries

int main(int argc, char **argv) {
  FILE *f;
  char st1[max_size];
  char bestw[N][max_size];
  char file_name[max_size], st[100][max_size];
  double dist, bestd[N];
  float len;
  long long words, size, a, b, c, d, cn, bi[100];
  float *M;
  char *vocab;
  if (argc < 2) {
    printf("Usage: ./word-analogy <FILE>\nwhere FILE contains word projections in the BINARY FORMAT\n");
    return 0;
  }
  strcpy(file_name, argv[1]);
  f = fopen(file_name, "rb");
  if (f == NULL) {
    printf("Input file not found\n");
    return -1;
  }
  fscanf(f, "%lld", &words);
  fscanf(f, "%lld", &size);
  vocab = (char *)malloc((long long)words * max_w * sizeof(char));
  M = (float *)malloc((long long)words * (long long)size * sizeof(float));
  if (M == NULL) {
    printf("Cannot allocate memory: %lld MB    %lld  %lld\n", (long long)words * size * sizeof(float) / 1048576, words, size);
    return -1;
  }
  for (b = 0; b < words; b++) {
    a = 0;
    while (1) {
      vocab[b * max_w + a] = fgetc(f);
      if (feof(f) || (vocab[b * max_w + a] == ' ')) break;
      if ((a < max_w) && (vocab[b * max_w + a] != '\n')) a++;
    }
    vocab[b * max_w + a] = 0;
    for (a = 0; a < size; a++) fread(&M[a + b * size], sizeof(float), 1, f);
    len = 0;
    for (a = 0; a < size; a++) len += M[a + b * size] * M[a + b * size];
    len = sqrt(len);
    for (a = 0; a < size; a++) M[a + b * size] /= len;
  }
  fclose(f);
  while (1) {
    for (a = 0; a < N; a++) bestd[a] = 0;
    for (a = 0; a < N; a++) bestw[a][0] = 0;
    printf("Enter three words (EXIT to break): ");
    a = 0;
    while (1) {
      st1[a] = fgetc(stdin);
      if ((st1[a] == '\n') || (a >= max_size - 1)) {
        st1[a] = 0;
        break;
      }
      a++;
    }
    if (!strcmp(st1, "EXIT")) break;
    cn = 0;
    b = 0;
    c = 0;
    while (1) {
      st[cn][b] = st1[c];
      b++;
      c++;
      st[cn][b] = 0;
      if (st1[c] == 0) break;
      if (st1[c] == ' ') {
        cn++;
        b = 0;
        c++;
      }
    }
    cn++;
    if (cn < 3) {
      printf("Only %lld words were entered.. three words are needed at the input to perform the calculation\n", cn);
      continue;
    }
    for (a = 0; a < cn; a++) {
      for (b = 0; b < words; b++) if (!strcmp(&vocab[b * max_w], st[a])) break;
      if (b == words) b = 0;
      bi[a] = b;
      printf("\nWord: %s  Position in vocabulary: %lld\n", st[a], bi[a]);
      
      if (b == 0) {
        printf("Out of dictionary word!\n");
        break;
      }
    }
    if (b == 0) continue;
    printf("\n                                              Word              Distance\n------------------------------------------------------------------------\n");


    // for(a = 0; a < size; a++) {
    //   printf("%f, ", M[a + bi[2] * size]);
    // }

    // Create an empty top N list that we'll output later
    for (a = 0; a < N; a++) bestd[a] = 0;
    for (a = 0; a < N; a++) bestw[a][0] = 0;

    // Loop over all words to see how closely related they are to the perfect oucome
    for (c = 0; c < words; c++) {
      if (c == bi[0]) continue;
      if (c == bi[1]) continue;
      if (c == bi[2]) continue;
      a = 0;
      for (b = 0; b < cn; b++) if (bi[b] == c) a = 1;
      if (a == 1) continue;
      dist = 0;
      
      // This is where the magic is happening: loop over all fields of the vector
      // Calculate similarity between vector V1={v_1_1, v_1_2, ...} and V2={v_2_1, v_2_2, ...} as:
      // SUM(v_1_i * v_2_i)
      for (a = 0; a < size; a++) {
        // Similarity measure: cos (b', b) − cos (b', a) + cos (b', a')
        // With cos(u, v) = (u*v)/(|u| * |v|)
        // Since u and v are normalized, cos(u, v) = u*v
        // This is referred to as the 2CosAdd method in the paper by Omer Levy and Yoav Goldberg (2014)
        // dist += M[a + c * size] * M[a + bi[2] * size] - M[a + c * size] * M[a + bi[0] * size] + M[a + c * size] * M[a + bi[1] * size];

        // (cos (b', b) cos (b', a') ) / cos(b',a)+ε
        dist += (
          ((double) M[a + c * size]) * ((double) M[a + bi[2] * size]) * 
          ((double) M[a + c * size]) * ((double) M[a + bi[0] * size])
        ) / (
          ((double) M[a + c * size]) * ((double) M[a + bi[1] * size]) + 0.001
        );
        // printf(" %f,\n", (M[a + c * size] * M[a + bi[2] * size] * M[a + c * size] * M[a + bi[1] * size]) / (M[a + c * size] * M[a + bi[0] * size] + 0.001f));
      }
      // return 1;

      for (a = 0; a < N; a++) {
        // higher score = better match
        if (dist > bestd[a]) {
          // Shift down everything that should be below this word
          for (d = N - 1; d > a; d--) {
            bestd[d] = bestd[d - 1];
            strcpy(bestw[d], bestw[d - 1]);
          }
          // And set this word in the empty place
          bestd[a] = dist;
          strcpy(bestw[a], &vocab[c * max_w]);
          break;
        }
      }
    }
    for (a = 0; a < N; a++) printf("%50s\t\t\t%f\n", bestw[a], bestd[a]);
  }
  return 0;
}
