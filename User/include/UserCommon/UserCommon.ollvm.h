#pragma once

#if CONFIG_OLLVM_ON == 1
#define CODE_NOTOBF   __attribute((__annotate__(("notobf"))))
#define CODE_NOTINDGV __attribute((__annotate__(("notindgv"))))

#define CODE_OBF __attribute((__annotate__(("needobf"))))
#define CODE_HIBR_ENC __attribute((__annotate__(("hisbr hiencbr fbbfenc"))))
#define CODE_OBF_FLT __attribute((__annotate__(("needobf flt"))))
#define CODE_OBF_MBA_FAST __attribute((__annotate__(("needobf const_subs"))))
#define CODE_OBF_MBA_ONEBIT __attribute((__annotate__(("needobf const_enc_subs"))))
#define CODE_OBF_MBA_TWOBIT __attribute((__annotate__(("needobf const_enc_ex_subs"))))

#define CODE_OBF_BB2F __attribute((__annotate__(("needobf const_subs bb2func"))))
#define CODE_OBF_MFLT __attribute((__annotate__(("needobf const_subs flt"))))
#define CODE_OBF_FLA  __attribute((__annotate__(("needobf fla"))))
#define CODE_OBF_MAX __attribute((__annotate__(("needobf split flt const_enc_ex_subs indgv indbr indcall"))))

#define CODE_OBF_LITE __attribute((__annotate__(("needobf fbbfenc indcall indgv hibr hisbr hiencbr smvm idgvlv1 idclv1 notindbr")))) //mba_subs

#define CODE_OBF_LITE2 __attribute((__annotate__(("needobf svm notindbr")))) //mba_subs

#define CODE_OBF_SVM __attribute((__annotate__(("needobf svm"))))

#define CODE_OBF_FBBENC __attribute((__annotate__(("needobf fbbfenc"))))

#define CODE_OBF_SMVM __attribute((__annotate__(("needobf smvm"))))
#else
#define CODE_NOTOBF
#define CODE_NOTINDGV

#define CODE_OBF
#define CODE_OBF_FLT
#define CODE_OBF_MBA_FAST
#define CODE_OBF_MBA_ONEBIT
#define CODE_OBF_MBA_TWOBIT

#define CODE_OBF_BB2F
#define CODE_OBF_MFLT
#define CODE_OBF_FLA
#define CODE_OBF_MAX

#define CODE_OBF_LITE
#define CODE_OBF_FBBENC
#define CODE_OBF_SMVM
#define CODE_OBF_SVM

#define CODE_OBF_LITE2
#endif // CONFIG_OLLVM_ON