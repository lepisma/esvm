#include <emacs-module.h>
#include "svm.h"

int plugin_is_GPL_compatible;

static emacs_value
Fmat_dot (emacs_env *env, ptrdiff_t nargs, emacs_value args[], void *data) {
  emacs_value va = args[0];
  emacs_value vb = args[1];
  ptrdiff_t vsize = env->vec_size(env, va);


  int sum = 0;
  for (int i = 0; i < vsize; i++) {
    sum += env->extract_integer(env, env->vec_get(env, va, i)) *  \
      env->extract_integer(env, env->vec_get(env, vb, i));
  }
  return env->make_integer(env, libsvm_version);
}

static void
bind_function (emacs_env *env, const char *name, emacs_value Sfun) {
  emacs_value Qfset = env->intern(env, "fset");
  emacs_value Qsym = env->intern(env, name);
  emacs_value args[] = { Qsym, Sfun };
  env->funcall(env, Qfset, 2, args);
}

static void
provide (emacs_env *env, const char *feature) {
  emacs_value Qfeat = env->intern(env, feature);
  emacs_value Qprovide = env->intern(env, "provide");
  emacs_value args[] = { Qfeat };

  env->funcall(env, Qprovide, 1, args);
}

int
emacs_module_init (struct emacs_runtime *ert) {
  emacs_env *env = ert->get_environment(ert);
  emacs_value fun = env->make_function(env, 2, 2, Fmat_dot, "Lol kek", NULL);

  bind_function(env, "mat-dot", fun);
  provide(env, "mat");

  return 0;
}
