/* Japanese messages are separate to another file.             */
/* Because Japanese EUC-JP encoding is conflict with ISO-8859. */
/* By: YOKOTA Hiroshi <yokota@netlab.is.tsukuba.ac.jp>         */

#ifndef __DCLOCK_MSG_JP_H__
#define __DCLOCK_MSG_JP_H__

#define POPEX_STRING      "���ߤ������͸�"
#define PEOPLE_STRING     " ��"
#define FOREST_STRING     "���ߤ�Ǯ�ӱ��Ӥ�����"
#define TROPICAL_STRING   " "
#define HIV_STRING        "���ߤ� HIV ������"
#define CASES_STRING      " ��"
#define LAB_STRING        "��ǯ��ưʪ�¸��ˤ��"
#define VEG_STRING        "��ǯ���ʹ֤ο��ȤȤ���"
#define YEAR_STRING       " �Τ�ưʪ�������ˤʤ�ޤ���"
#define Y2K_STRING        "Y2K (2000ǯ1��1��, 0��00ʬ00��) �ޤ� ����"
#define POST_Y2K_STRING   "Y2K (2000ǯ1��1��, 0��00ʬ00��) ����"
#define Y2001_STRING      "����ǯ���ν�λ (2001ǯ1��1��, 0��00ʬ00��) �ޤ� ����"
#define POST_Y2001_STRING "����ǯ���γ��� (2001ǯ1��1��, 0��00ʬ00��) ����"
#define DAY               "��"
#define DAYS              "��"
#define HOUR              "����"
#define HOURS             "����"
#define MINUTE            "ʬ"
#define MINUTES           "ʬ"
#define SECOND            "��"
#define SECONDS           "��"

#ifndef METRIC
#define METRIC 1
#endif

#if METRIC
#define AREA_STRING "�إ�������"
#else
#define AREA_STRING "��������"
#endif

#endif
