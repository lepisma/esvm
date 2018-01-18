#include <stdlib.h>
#include "emacs-module.h"
#include "libsvm/svm.h"

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

static double
vref_f (emacs_env *env, emacs_value vec, int i)
{
  return env->extract_float(env, vref(env, vec, i));
}

static emacs_value
vref2 (emacs_env *env, emacs_value vec, int i, int j)
{
  return vref(env, vref(env, vec, i), j);
}

static double
vref2_f (emacs_env *env, emacs_value vec, int i, int j)
{
  return env->extract_float(env, vref2(env, vec, i, j));
}

static emacs_value
make_vector (emacs_env *env, int len, double init)
{
  emacs_value Fmake_vector = env->intern(env, "make-vector");
  emacs_value args[] = { env->make_integer(env, len), env->make_float(env, init) };
  return env->funcall(env, Fmake_vector, 2, args);
}

static void
fin_model (void *model)
{
  free(model);
}

static emacs_value
Ffit (emacs_env *env, ptrdiff_t n, emacs_value args[], void *data)
{
  struct svm_parameter parameters;
  struct svm_problem problem;
  struct svm_model *model;
  struct svm_node *train_nodes;

  emacs_value x_train = args[0];
  emacs_value y_train = args[1];

  int train_size = env->vec_size(env, x_train);
  int n_features = env->vec_size(env, vref(env, x_train, 0));

  problem.l = train_size;
  problem.y = malloc(train_size * sizeof(double));
  problem.x = malloc(train_size * sizeof(struct svm_node *));
  train_nodes = malloc(train_size * (n_features + 1) * sizeof(struct svm_node));

  for (int i = 0; i < problem.l; i++)
    {
      problem.y[i] = vref_f(env, y_train, i);
    }

  int j = 0;
  for (int i = 0; i < problem.l; i++)
    {
      problem.x[i] = &train_nodes[j];
      for (int k = 0; k < n_features; k++, j++)
        {
          train_nodes[j].index = k + 1;
          train_nodes[j].value = vref2_f(env, x_train, i, k);
        }
      train_nodes[j].index = -1;
      train_nodes[j].value = 0;
      j++;
    }

  parameters.svm_type = C_SVC;
  parameters.kernel_type = RBF;
  parameters.degree = 3;
  parameters.gamma = 1.0 / n_features;
  parameters.coef0 = 0;
  parameters.nu = 0.5;
  parameters.cache_size = 100;
  parameters.C = 1;
  parameters.eps = 1e-3;
  parameters.p = 0.1;
  parameters.shrinking = 1;
  parameters.probability = 0;
  parameters.nr_weight = 0;
  parameters.weight_label = NULL;
  parameters.weight = NULL;

  model = svm_train(&problem, &parameters);
  return env->make_user_ptr(env, fin_model, model);
}

static emacs_value
Fpredict (emacs_env *env, ptrdiff_t n, emacs_value args[], void *data)
{
  struct svm_model *model = env->get_user_ptr(env, args[0]);

  emacs_value x_test = args[1];
  int test_size = env->vec_size(env, x_test);
  int n_features = env->vec_size(env, vref(env, x_test, 0));

  emacs_value y_out = make_vector(env, test_size, 0.0);

  // Predict
  int j = 0;
  struct svm_node *test_nodes = malloc((n_features + 1) * sizeof(struct svm_node));
  for (int i = 0; i < test_size; i++)
    {
      for (j = 0; j < n_features; j++)
        {
          test_nodes[j].index = j + 1;
          test_nodes[j].value = vref2_f(env, x_test, i, j);
        }
      test_nodes[j].index = -1;
      test_nodes[j].value = 0;
      env->vec_set(env, y_out, i, env->make_float(env, svm_predict(model, test_nodes)));
    }

  free(test_nodes);

  return y_out;
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

  emacs_value fitfun = env->make_function(env, 2, 2, Ffit, "Train on data and return model", NULL);
  bind_function(env, "esvm--fit", fitfun);

  emacs_value predictfun = env->make_function(env, 2, 2, Fpredict, "Return prediction for given model and data", NULL);
  bind_function(env, "esvm--predict", predictfun);

  provide(env, "esvm-core");

  return 0;
}
