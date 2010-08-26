#include "result.h"

VALUE cSwiftResult;

void result_free(dbi::AbstractResultSet *result) {
  if (result) {
    result->cleanup();
    delete result;
  }
}

// TODO:
static VALUE result_clone(VALUE self) {
  rb_raise(eRuntimeError, "clone is not allowed.");
}

// TODO:
static VALUE result_dup(VALUE self) {
  rb_raise(eRuntimeError, "dup is not allowed.");
}

VALUE result_each(VALUE self) {
  ulong length;
  const char *data;

  dbi::AbstractStatement *handle = result_handle(self);
  VALUE adapter                  = rb_iv_get(self, "@adapter");
  VALUE scheme                   = rb_iv_get(self, "@scheme");

  try {
    VALUE fields                      = rb_ary_new();
    std::vector<string> handle_fields = handle->fields();
    std::vector<int>    handle_types  = handle->types();
    for (uint i = 0; i < handle_fields.size(); i++) {
      rb_ary_push(fields, ID2SYM(rb_intern(handle_fields[i].c_str())));
    }

    handle->seek(0);
    for (uint row = 0; row < handle->rows(); row++) {
      VALUE tuple = rb_hash_new();
      for (uint column = 0; column < handle->columns(); column++) {
        data = (const char*)handle->read(row, column, &length);
        if (data) {
          rb_hash_aset(
            tuple,
            rb_ary_entry(fields, column),
            typecast_field(adapter, handle_types[column], data, length)
          );
        }
        else {
          rb_hash_aset(tuple, rb_ary_entry(fields, column), Qnil);
        }
        NIL_P(scheme) ? rb_yield(tuple) : rb_yield(rb_funcall(scheme, rb_intern("load"), 1, tuple));
      }
    }
  }
  CATCH_DBI_EXCEPTIONS();

  return Qnil;
}

void init_swift_result() {
  rb_require("bigdecimal");
  rb_require("stringio");

  VALUE swift  = rb_define_module("Swift");
  cSwiftResult = rb_define_class_under(swift, "Result", rb_cObject);

  rb_define_alloc_func(cSwiftResult, result_alloc);
  rb_include_module(cSwiftResult, CONST_GET(rb_mKernel, "Enumerable"));

  rb_define_method(cSwiftResult, "clone",      RUBY_METHOD_FUNC(result_clone),     0);
  rb_define_method(cSwiftResult, "dup",        RUBY_METHOD_FUNC(result_dup),       0);
  rb_define_method(cSwiftResult, "each",       RUBY_METHOD_FUNC(result_each),      0);
  rb_define_method(cSwiftResult, "execute",    RUBY_METHOD_FUNC(result_execute),   -1);
  rb_define_method(cSwiftResult, "finish",     RUBY_METHOD_FUNC(result_finish),    0);
  rb_define_method(cSwiftResult, "initialize", RUBY_METHOD_FUNC(result_init),      2);
  rb_define_method(cSwiftResult, "insert_id",  RUBY_METHOD_FUNC(result_insert_id), 0);
  rb_define_method(cSwiftResult, "read",       RUBY_METHOD_FUNC(result_read),      0);
  rb_define_method(cSwiftResult, "rewind",     RUBY_METHOD_FUNC(result_rewind),    0);
  rb_define_method(cSwiftResult, "rows",       RUBY_METHOD_FUNC(result_rows),      0);
}

/*
  TODO:
  * Parse components in C. Exactly what sorts of formats need to be parsed by this?
  * Perfer embedded > adapter > local timezone.
*/
VALUE typecast_datetime(VALUE adapter, const char *data, ulong length) {
  return rb_funcall(CONST_GET("DateTime", rb_mKernel), rb_intern("iso8601"), 1, rb_str_new(data, length));
}

VALUE typecast_field(VALUE adapter, int type, const char *data, ulong length) {
  switch(type) {
    case DBI_TYPE_BOOLEAN:
      return strcmp(data, "t") == 0 || strcmp(data, "1") == 0 ? Qtrue : Qfalse;
    case DBI_TYPE_INT:
      return rb_cstr2inum(data, 10);
    case DBI_TYPE_BLOB:
      return rb_funcall(CONST_GET("StringIO", rb_mKernel), rb_intern("new"), 1, rb_str_new(data, length));
    case DBI_TYPE_TEXT:
      // Forcing UTF8 convention here. Do we really care about people using non utf8 client encodings and databases?
      return rb_enc_str_new(data, length, rb_utf8_encoding());
    case DBI_TYPE_TIME:
      return result_typecast_datetime(self, data, length);
    case DBI_TYPE_NUMERIC:
      return rb_funcall(CONST_GET("BigDecimal", rb_mKernel), rb_intern("new"), 1, rb_str_new2(data));
    case DBI_TYPE_FLOAT:
      return rb_float_new(atof(data));
  }
}

