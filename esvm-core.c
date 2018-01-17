#include <stdlib.h>
#include "emacs-module.h"
#include "svm.h"

int plugin_is_GPL_compatible;

static emacs_value
Flibsvm_version (emacs_env *env, ptrdiff_t n, emacs_value args[], void *data)
{
  return env->make_integer(env, libsvm_version);
}

static emacs_value
vref (emacs_env *env, emacs_value vec, int i)
{
  return env->vec_get(env, vec, i);
}

static float
vref_f (emacs_env *env, emacs_value vec, int i)
{
  return env->extract_float(env, vref(env, vec, i));
}

static emacs_value
vref2 (emacs_env *env, emacs_value vec, int i, int j)
{
  return vref(env, vref(env, vec, i), j);
}

static float
vref2_f (emacs_env *env, emacs_value vec, int i, int j)
{
  return env->extract_float(env, vref2(env, vec, i, j));
}

static emacs_value
Fgo (emacs_env *env, ptrdiff_t n, emacs_value args[], void *data)
{
  struct svm_parameter params;
  struct svm_problem prob;
  struct svm_model *model;
  struct svm_node *x_space;

  emacs_value x_train = args[0];
  emacs_value y_train = args[1];
  emacs_value x_test = args[2];
  emacs_value y_out = args[3];

  int problem_size = env->vec_size(env, y_train);
  int features = env->vec_size(env, vref(env, x_train, 0));

  prob.l = problem_size;
  prob.y = (double *) malloc(problem_size * sizeof(double));
  prob.x = (struct svm_node **) malloc(problem_size * sizeof(struct svm_node *));
  x_space = (struct svm_node *) malloc(problem_size * (features + 1) * sizeof(struct svm_node));

  for (int i = 0; i < prob.l; i++)
    {
      prob.y[i] = vref_f(env, y_train, 0);
    }

  int j = 0;
  for (int i = 0; i < prob.l; i++)
    {
      prob.x[i] = &x_space[j];
      for (int k = 0; k < features; ++k, ++j) {
        x_space[j].index = k + 1;
        x_space[j].value = vref2_f(env, x_train, i, k);
      }
      x_space[j].index = -1;
      x_space[j].value = 0;
      j++;
    }

  params.svm_type = C_SVC;
  params.kernel_type = RBF;
  params.degree = 3;
  params.gamma = 0;
  params.coef0 = 0;
  params.nu = 0.5;
  params.cache_size = 100;
  params.C = 1;
  params.eps = 1e-3;
  params.p = 0.1;
  params.shrinking = 1;
  params.probability = 0;
  params.nr_weight = 0;
  params.weight_label = NULL;
  params.weight = NULL;

  model = svm_train(&prob, &params);

  // Predict
  struct svm_node *x_space_test = (struct svm_node *) malloc((features + 1) * sizeof(struct svm_node));

  int ntest = env->vec_size(env, y_out);
  for (int i = 0; i < ntest; i++) {
    int j;
    for (j = 0; j < features; j++) {
      x_space_test[j].index = j + 1;
      x_space_test[j].value = vref2_f(env, x_test, i, j);
    }
    x_space_test[j].index = -1;
    x_space_test[j].value = 0;
    env->vec_set(env, y_out, j, env->make_float(env, svm_predict(model, x_space_test)));
  }

  return env->make_string(env, "done", 4);
}

static void
bind_function (emacs_env *env, const char *name, emacs_value Sfun)
{
  emacs_value Qfset = env->intern(env, "fset");
  emacs_value Qsym = env->intern(env, name);
  emacs_value args[] = { Qsym, Sfun };
  env->funcall(env, Qfset, 2, args);
}

static void
provide (emacs_env *env, const char *feature)
{
  emacs_value Qfeat = env->intern(env, feature);
  emacs_value Qprovide = env->intern(env, "provide");
  emacs_value args[] = { Qfeat };

  env->funcall(env, Qprovide, 1, args);
}

int
emacs_module_init (struct emacs_runtime *ert)
{
  emacs_env *env = ert->get_environment(ert);

  emacs_value vfun = env->make_function(env, 0, 0, Flibsvm_version, "Return the version of libsvm", NULL);
  bind_function(env, "esvm--libsvm-version", vfun);

  emacs_value gfun = env->make_function(env, 4, 4, Fgo, "Go go go", NULL);
  bind_function(env, "esvm--go", gfun);

  provide(env, "esvm-core");

  return 0;
}
