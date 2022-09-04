#include <stdio.h>

#include <dir.h>
#include <array.h>
#include <terminal.h>
#include <file.h>
#include <stream.h>
#include <str.h>

int main(int argc, char *argv[])
{
  Array *items;
  
  if (argc < 2) items = directory(".");
  else          items = directory(argv[1]);

  for (int i = 0; i < items->size; i++) {
    DirectoryItem *di = *(DirectoryItem**)at(items, i);

    if (di->type == DIRITEM_FILE) {
      Stream *file = fromFileStream(fopen(di->name, "r"));

      if (file) {
        String *line;

        while ((line = sgetl(file))) {
          String *todo  = newString("TODO:");
          String *colon = newString(":");
          int     index;

          if ((index = contains(line, todo)) > -1) {
            String *prior = substring(newString(line->content), index + 6, 0);
            String *what;

            substring(prior, 0, contains(prior, colon));
            what = substring(newString(line->content), prior->length + index + 7, 0);

            printf("%s: %s\n", prior->content, what->content);

            deleteString(&prior);
            deleteString(&what);
          }

          deleteString(&todo);
          deleteString(&colon);
          deleteString(&line);
        }
      }

      sclose(file);
    } else if (di->type == DIRITEM_DIRECTORY) {
      char *argv[2] = { "", di->name };
      main(2, argv);
    }
  }

  return 0;
}