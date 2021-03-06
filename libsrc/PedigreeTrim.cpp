////////////////////////////////////////////////////////////////////// 
// libsrc/PedigreeTrim.cpp 
// (c) 2000-2010 Goncalo Abecasis
// 
// This file is distributed as part of the Goncalo source code package   
// and may not be redistributed in any form, without prior written    
// permission from the author. Permission is granted for you to       
// modify this file for your own personal use, but modified versions  
// must retain this copyright notice and must not be distributed.     
// 
// Permission is granted for you to use this file to compile Goncalo.    
// 
// All computer programs have bugs. Use this file at your own risk.   
// 
// Monday February 08, 2010
// 
 
#include "Pedigree.h"

void Pedigree::ShowTrimHeader(bool & flag)
   {
   if (flag)
      {
      printf("Trimming uninformative individuals...\n");
      flag = false;
      }
   }

void Pedigree::Trim(bool quiet, int * informative)
   {
   int       newCount = 0;
   Person ** newPersons = new Person * [count];

   // This function applies the following filters to reduce complexity
   // of pedigree
   //
   // RULE 1:         Remove all pedigrees no genotype or phenotype data
   // RULE 2:         Remove leaf individuals with no data
   // RULE 3:         Remove founder couples with <2 offspring and no data

   bool     showHeader = true;
   IntArray discardable, offspring, mates, haveData;

   for (int f = 0; f < familyCount; f++)
      {
      Family * fam = families[f];

      // Cache for storing indicators about whether each family member is
      // informative
      haveData.Dimension(fam->count);

      // Check that some data is available in the family
      int hasData = false;
      for (int i = fam->first; i <= fam->last; i++)
         if (informative == NULL)
            hasData |= haveData[persons[i]->traverse] = persons[i]->haveData();
         else
            hasData |= haveData[persons[i]->traverse] = informative[i];

      if (!hasData)
         {
         if (!quiet)
            {
            ShowTrimHeader(showHeader);
            printf("   Removing family %s: No data\n", (const char *) fam->famid);
            }

         for (int i = fam->first; i <= fam->last; i++)
            delete persons[i];

         continue;
         }

      // Assume that we need everyone in the family
      discardable.Dimension(fam->count);
      discardable.Set(0);

      bool trimming = true;

      while (trimming)
         {
         trimming = false;

         // Tally the number of offspring for each individual
         offspring.Dimension(fam->count);
         offspring.Zero();

         // Tally the number of mates for each individual
         mates.Dimension(fam->count);
         mates.Set(-1);

         // In the first round, we count the number of offspring
         // for each individual in the current trimmed version of the
         // pedigree
         for (int i = fam->count - 1; i >= fam->founders; i--)
            {
            if (discardable[i]) continue;

            Person & p = *(persons[fam->path[i]]);

            if (discardable[p.father->traverse])
               continue;

            if (offspring[i] == 0 && !haveData[p.traverse])
               {
               trimming = true;
               discardable[i] = true;
               continue;
               }

            int father = p.father->traverse;
            int mother = p.mother->traverse;

            if (mates[father] == -1 && mates[mother] == -1)
               {
               mates[father] = mother,
               mates[mother] = father;
               }
            else if (mates[father] != mother)
               {
               if (mates[father] >= 0)
                 mates[mates[father]] = -2;

               if (mates[mother] >= 0)
                 mates[mates[mother]] = -2;

               mates[mother] = -2;
               mates[father] = -2;
               }

            offspring[father]++;
            offspring[mother]++;
            }

         // In the second pass, we remove individuals with no
         // data who are founders with a single offspring (and
         // no multiple matings) or who have no descendants
         for (int i = fam->count - 1; i >= 0; i--)
            {
            if (discardable[i]) continue;

            Person & p = *(persons[fam->path[i]]);

            if (p.isFounder() || discardable[p.father->traverse])
               {
               if (mates[i] == -2 ||
                   offspring[i] > 1 ||
                   mates[i] >= fam->founders &&
                   !discardable[persons[fam->path[mates[i]]]->father->traverse] ||
                   haveData[p.traverse] ||
                   mates[i] != -1 && haveData[mates[i]])
                   continue;

               trimming = true;
               discardable[i] = true;
               continue;
               }
            }
         }

      for (int i = fam->count - 1; i >= 0; i--)
         if (discardable[i])
            {
            if (!quiet)
               {
               ShowTrimHeader(showHeader);
               printf("   Removing person %s->%s: No data\n",
                   (const char *) fam->famid,
                   (const char *) persons[fam->path[i]]->pid);
               }
            delete persons[fam->path[i]];
            }
         else
            newPersons[newCount++] = persons[fam->path[i]];
      }

   if (!showHeader)
      printf("\n");

   delete [] persons;

   persons = newPersons;
   count = newCount;
   Sort();
   }


 
