////////////////////////////////////////////////////////////////////// 
// libsrc/InputFile.cpp 
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
 
#include "InputFile.h"
#include "StringBasics.h"

#include <stdarg.h>

#ifdef __ZLIB_AVAILABLE__

IFILE::IFILE(const char * filename, const char * mode)
   {
   // Some implementations of zlib will not open files that are
   // larger than 2Gb. To ensure support for large (uncompressed)
   // files, we fall-back on the regular fopen when the initial
   // gzopen call fails and the filename does not end in .gz

   gzMode = true;
   gzHandle = gzopen(filename, mode);

   if (gzHandle == NULL)
      {
      int lastchar = 0;

      while (filename[lastchar] != 0) lastchar++;

      if (lastchar >= 3 && filename[lastchar - 3] == '.' &&
                           filename[lastchar - 2] == 'g' &&
                           filename[lastchar - 1] == 'z')
         return;

      gzMode = false;
      handle = fopen(filename, mode);
      }
   };

#endif

int ifprintf(IFILE output, char * format, ...)
   {
#ifdef __ZLIB_AVAILABLE__
   if (output.gzMode == true)
      {
      String buffer;

      va_list  ap;
      va_start(ap, format);

      buffer.vprintf(format, ap);

      va_end(ap);

      return gzwrite(output.gzHandle, (const char *) buffer, buffer.Length());
      }
#endif

   va_list  ap;
   va_start(ap, format);

   int result = vfprintf(output.handle, format, ap);

   va_end(ap);

   return result;
   }

 
