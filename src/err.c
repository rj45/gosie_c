#include "gosie.h"

void errInit(ErrorList *err, Source src) { *err = (ErrorList){.src = src}; }

void errFree(ErrorList *err) {
  for (size_t i = 0; i < arrlen(err->errors); i++) {
    free((void *)err->errors[i].msg);
  }
  arrfree(err->errors);
}

bool errHasErrors(ErrorList *err) { return arrlen(err->errors) > 0; }

void errErrorf(ErrorList *err, Token token, const char *msg, ...) {
  va_list ap;
  va_start(ap, msg);

  char *buf = NULL;
  vasprintf(&buf, msg, ap);
  va_end(ap);

  Error error = (Error){.msg = buf, .token = token};
  arrput(err->errors, error);
}

// TODO: upgrade with line numbers once we have more than one line
// TODO: upgrade with file names once we have more than one file
void errPrintAll(ErrorList *err) {
  for (size_t i = 0; i < arrlen(err->errors); i++) {
    Error error = err->errors[i];
    fprintf(stderr, "%d: error: %s\n", error.token.position, error.msg);
  }
}