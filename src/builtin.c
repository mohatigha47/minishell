/*
  Projet minishell - Licence 3 Info - PSI 2023

  Nom : SIALE-KOYAMBA
  Prénom : Ephraïm
  Num. étudiant : 22313533
  Groupe de projet : Binome 2
  Date : 22/11/2023

  Gestion des commandes internes du minishell (implémentation).

 */

#include "cmd.h"
#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int is_builtin(const char *cmd)
{
  if (strcmp(cmd, "cd") == 0 || strcmp(cmd, "exit") == 0 || strcmp(cmd, "unset") == 0  || strcmp(cmd, "echo") == 0 || strcmp(cmd, "export") == 0)
  {
    return 1;
  }
  return 0;
}

int builtin(cmd_t *cmd)
{
  if (strcmp(cmd->path, "cd") == 0)
  {
    if (cmd->argv[1] == NULL)
    {

      return cd(getenv("HOME"), STDERR_FILENO); // Changer vers le répertoire standard "HOME" si pas d'arguments
    }
    else
    {
      return cd(cmd->argv[1], STDERR_FILENO); // Changer vers le répertoire specifié
    }
  }
  else if (strcmp(cmd->path, "export") == 0)
  {
    if (cmd->argv[1] != NULL && cmd->argv[2] != NULL)
    {
      return export(cmd->argv[1], cmd->argv[2], STDERR_FILENO); // Mettre le variable d'environement
    }
    else
    {
      fprintf(stderr, "Utilisation: export VAR VALEUR\n");
      return -1;
    }
  }
  else if (strcmp(cmd->path, "echo") == 0)
  {

    // écrire tous les arguments aprés token "echo"
    for (int i = 1; cmd->argv[i] != NULL; i++)
    {
      printf("%s", cmd->argv[i]);
      if (cmd->argv[i + 1] != NULL)
        printf(" ");
    }
    printf("\n");

    return 0; 
  }
  else if (strcmp(cmd->path, "unset") == 0)
  {
    // Vérifier le nombre d'arguments
    if (cmd->argv[1] != NULL && cmd->argv[2] == NULL)
    {
      int status = unset_variable(cmd->argv[1]);
      return status;
    }
    else
    {
      fprintf(stderr, "Utilisation: unset VAR\n");
      return -1;
    }
  }
  else if (strcmp(cmd->path, "exit") == 0)
  {
    int exit_code = 0;
    if (cmd->argv[1] != NULL)
    {
      exit_code = atoi(cmd->argv[1]);
    }
    exit_shell(exit_code);
  }
  return -1;
}

/*
Fonction unset_variable
Paramètre a : charactère permettant de passer la valeur de variable d'environnement
Enlever un variable d'environement (unset)
*/

int unset_variable(char *var)
{
  int status = unsetenv(var);
  if (status != 0)
  {
    fprintf(stderr, "Erreur: on ne peut pas enlever le variable d'environnement %s\n", var);
  }
  return status;
}

int cd(const char *path, int fderr)
{
  int status = chdir(path);
  if (status != 0)
  {
    dprintf(fderr, "Ereur: On ne peut pas changer le répertoire vers %s\n", path);
  }
  return status;
}

int export(const char *var, const char *value, int fderr)
{
  int status = setenv(var, value, 1);
  if (status != 0)
  {
    dprintf(fderr, "Ereur: On ne peut pas mettre le variable d'environement %s\n", var);
  }
  return status;
}

int exit_shell(int ret)
{

  printf("Exiting shell with return code %d\n", ret);
  exit(ret);
}
