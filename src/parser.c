/*
  Projet minishell - Licence 3 Info - PSI 2023

  Nom : SIALE-KOYAMBA
  Prénom : Ephraïm
  Num. étudiant : 22313533
  Groupe de projet : Binome 2
  Date : 22/11/2023

  Parsing de la ligne de commandes utilisateur (implémentation).

 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int trim(char *str)
{
  if (str == NULL)
  {
    return -1;
  }

  size_t len = strlen(str);
  if (len == 0)
  {
    return 0;
  }

  // Suprimmer les espaces au début
  size_t start = 0;
  while (isspace((unsigned char)str[start]))
  {
    start++;
  }

  // Si la chaine est blanc
  if (start == len)
  {
    str[0] = '\0';
    return 0;
  }

  // suprimmer les espaces à la fin
  size_t end = len - 1;
  while (end > start && isspace((unsigned char)str[end]))
  {
    end--;
  }

  // Décalage vers le début
  memmove(str, &str[start], end - start + 1);
  str[end - start + 1] = '\0';

  return 0;
}

int clean(char *str)
{
  if (str == NULL)
  {
    return -1;
  }

  size_t len = strlen(str);

  size_t j = 0; 

  for (size_t i = 0; i < len; ++i)
  {
    if (isspace((unsigned char)str[i]))
    {
      if (i > 0 && isspace((unsigned char)str[i - 1]))
      {
        continue; 
      }
    }
    str[j++] = str[i];
  }

  str[j] = '\0'; 
  return 0;
}

int separate_s(char *str, char *s, size_t max)
{
}

int substenv(char *str, size_t max)
{
}

int strcut(char *str, char sep, char **tokens, size_t max)
{
  size_t tokenCount = 0;

  while (tokenCount < max)
  {
    // chercher la première lettre du token
    while (*str && *str == sep)
    {
      str++; 
    }

    if (*str == '\0')
    {
      break; // fin du chaine caractère
    }

    tokens[tokenCount] = str; // première lettre token
    tokenCount++;

    // chercher la fin du token
    while (*str && *str != sep)
    {
      str++;
    }

    if (*str == '\0')
    {
      break; // vérifier encore la fin du chaine
    }

    *str = '\0'; // ajouter le terminateur du token
    str++;
  }

  return tokenCount;
}

// Optionnel
int strcut_s(char *str, char sep, char **tokens, size_t max)
{
}
