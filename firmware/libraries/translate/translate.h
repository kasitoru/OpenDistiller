/*
    OpenDistiller Project
    Author: Sergey Avdeev (avdeevsv91@yandex.ru)
    URL: https://github.com/kasitoru/OpenDistiller
*/

#ifndef TRANSLATE_H_
#define TRANSLATE_H_

#ifndef LANG
    #define LANG RU
#endif

#define I18N_IMPL(S) #S
#define I18N_STR(S) I18N_IMPL(S)
#include I18N_STR(languages/LANG.h)

#endif /* TRANSLATE_H_ */
