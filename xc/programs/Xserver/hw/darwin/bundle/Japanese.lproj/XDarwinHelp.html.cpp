<!-- $XFree86: xc/programs/Xserver/hw/darwin/bundle/Japanese.lproj/XDarwinHelp.html.cpp,v 1.1.2.2 2001/05/22 03:36:53 torrey Exp $ -->

#include "xf86Version.h"
#ifndef PRE_RELEASE
#define PRE_RELEASE XF86_VERSION_SNAP
#endif

<html>
<head>
<META http-equiv="Content-Type" content="text/html; charset=EUC-JP">
<title>
XFree86 for Mac OS X</title></head>
<body>
<center>
    <h1>XFree86 on Darwin and Mac OS X</h1>
    XFree86 XF86_VERSION<br>
    Release Date: XF86_REL_DATE
</center>
<h2>�ܼ�</h2>
<ol>
    <li><A HREF="#notice">��ջ���</A></li>
    <li><A HREF="#usage">����ˡ</A></li>
    <li><A HREF="#path">�ѥ�������</A></li>
    <li><A HREF="#prefs">�Ķ�����</A></li>
    <li><A HREF="#license">�饤����</A></li>
</ol>
<center>
        <h2><a NAME="notice">��ջ���</a></h2>
</center>
<blockquote>
#if PRE_RELEASE
����ϡ�XFree86 �Υץ��꡼���С������Ǥ��ꡤ�����ʤ���ˤ����Ƥ⥵�ݡ��Ȥ���ޤ��� 
�Х�������ѥå��� SourceForge �� <A HREF="http://sourceforge.net/projects/xonx/">XonX �ץ������ȥڡ���</A>����Ф���Ƥ��뤫�⤷��ޤ���
�ץ��꡼���С������ΥХ�����𤹤����ˡ�<A HREF="http://sourceforge.net/projects/xonx/">XonX</A> �ץ������ȥڡ����ޤ��� <A HREF="http://www.XFree86.Org/cvs">XFree86 CVS ��ݥ��ȥ�</A>�Ǻǿ��ǤΥ����å��򤷤Ʋ�������
</blockquote>
#else
�⤷�������С��� 6 -12 ����ʾ����Τ�Τ����ޤ��Ϥ��ʤ��Υϡ��ɥ��������嵭�����դ��⿷������Τʤ�С��������𤹤����ˤ�꿷�����С�������õ���ƤߤƤ���������
�Х�������ѥå��� SourceForge �� <A HREF="http://sourceforge.net/projects/xonx/">XonX �ץ������ȥڡ���</A>����Ф���Ƥ��뤫�⤷��ޤ���
#endif
</blockquote>
<blockquote>
�ܥ��եȥ������ϡ�<A HREF="#license">MIT X11/X Consortium License</A> �ξ��˴�Ť���̵�ݾڤǡ��֤��Τޤޡפη��Ƕ��뤵��ޤ���
�����Ѥˤʤ����ˡ�<A HREF="#license">�饤���󥹾��</A>���ɤ߲�������
</blockquote>

<h2><a NAME="usage">����ˡ</a></h2>
<p>
XFree86 �ϡ�<a HREF="http://www.XFree86.Org/">XFree86 Project, Inc.</a>�ˤ�äƺ������줿�������۲�ǽ�ʥ����ץ󥽡����� <a HREF="http://www.x.org/">X Window System</a> �μ����Ǥ���
Mac OS X ��ǡ�XFree86 �ϥե륹���꡼��⡼�ɤ�ư��ޤ���X Window System �������ƥ��֤ʻ�������������̤���ͭ���ޤ���
���ʤ��ϡ�Command-Option-A �����򲡤����Ȥˤ�ä� Mac OS X �ǥ����ȥåפ��ڤ��ؤ��뤳�Ȥ��Ǥ��ޤ������Υ������Ȥ߹�碌�ϡ��Ķ�������ѹ���ǽ�Ǥ���
Mac OS X �ǥ����ȥåפ���X Window System ���ڤ��ؤ�����ϡ��ե��ƥ��󥰡�������ɥ���ɽ�����줿 XDarwin ��������򥯥�å����Ƥ���������
�ɥå���ɽ�����줿 XDarwin ��������Υ���å��� X Window System ���ڤ��ؤ��褦�ˡ��Ķ������ư����ѹ����뤳�Ȥ�Ǥ��ޤ���
</p>

<h3>ʣ���ܥ���ޥ����Υ��ߥ�졼�����</h3>
<p>
¿���� X11 ���ץꥱ�������ϡ�3 �ܥ���ޥ�����ɬ�פȤ��ޤ���
1 �ܥ���ޥ����� 3 �ܥ���ޥ����򥨥ߥ�졼����󤹤�ˤϡ��Ķ�����ǡ�ʣ���ܥ���ޥ����Υ��ߥ�졼������ͭ���ˤ���פ����򤷤ޤ���
����ϡ������ X �����С��ε�ư�����ͭ���Ȥʤ�ޤ���

3 �ܥ���ޥ����򥨥ߥ�졼����󤹤�������Υ��ޥ�ɥ����򲡤��ʤ���ޥ����ܥ���򥯥�å����뤳�Ȥ��� 2 �ޥ����ܥ���Υ���å����������ޤ������Υ��ץ���󥭡��򲡤��ʤ��饯��å����뤳�Ȥ��� 3 �ޥ����ܥ���Υ���å����������ޤ���
</p>
<p>��</p>
<ul>
    <li>¿���Υ����ܡ��ɤǤϡ������Υ��ޥ�ɥ����ȥ��ץ���󥭡��϶��̤��줺��Ʊ��ư��򤷤ޤ���
    <li>xmodmap �ǥ��ޥ�ɥ����䥪�ץ���󥭡���¾�Υ����˳�����ƤƤ�����Ǥ⡤ʣ���ܥ���ޥ����Υ��ߥ�졼�����Ǥ�����Υ��ޥ�ɥ����䥪�ץ���󥭡���Ȥ�ʤ���Фʤ�ޤ���
    <li>���Υ��ޥ�ɥ����򲡤��ʤ����� 2 �ޥ����ܥ���򥯥�å����뤳�Ȥ�¸�����ˤϡ����Υ��ޥ�ɥ�����¾�Υ����˳�����Ƥ�ɬ�פ�����ޤ������Υ��ץ���󥭡��򲡤��ʤ����� 3 �ޥ����ܥ���򥯥�å��������Ʊ�ͤǤ���
</ul>
<h2>
<a NAME="path">�ѥ�������</a>
</h2>
<p>
X11 �Х��ʥ�ϡ�/usr/X11R6/bin ���֤���ޤ������ʤ��Ϥ����ѥ��˲ä���ɬ�פ����뤫�⤷��ޤ���
�ѥ��ϡ� �¹Բ�ǽ�ʥ��ޥ�ɤ򸡺�����ǥ��쥯�ȥ�Υꥹ�ȤǤ���
������ǧ������ˡ�ϡ����Ѥ��Ƥ��륷����˰�¸���ޤ���
tcsh �Ǥϡ���printenv PATH�פȥ����פ��뤳�Ȥǥѥ�������å����뤳�Ȥ��Ǥ��ޤ���
�ѥ�������å���������/usr/X11R6/bin ���ǥ��쥯�ȥ�ΤҤȤĤȤ���ɽ������ʤ���Фʤ�ޤ���
�⤷�����Ǥʤ���С����ʤ��Ϥ����ǥե���ȤΥѥ����ɲä���ɬ�פ�����ޤ���
���Τ���ˡ�~/Library/init/tcsh/path �ե�����˼��ιԤ��ɲä��Ʋ�������
�ʤ⤷���Υե����뤬̵����к������Ʋ�������
</p>
<blockquote>
setenv PATH "${PATH}:/usr/X11R6/bin"
</blockquote>
<p>
�⤷�����ʤ��� .cshrc �ե�����ޤ��� .tcshrc �ե�������������ʤ�С������Υե������ ~/Library/init/tcsh/path �ե�����������ͤ��񤭤��ޤ���
�����ơ����ʤ��Ϥ����Υե�����Τ����ΰ�Ĥ������ѹ�����ɬ�פ����뤳�Ȥ���դ��Ʋ�������
�������ѹ��ϡ����ʤ��������� Terminal ������ɥ��򳫻Ϥ���ޤ�ͭ���Ȥʤ�ޤ���
�ޤ������ʤ��ϥɥ�����Ȥ�õ���Ƥ������XFree86 �Υޥ˥奢��ڡ����򸡺������ڡ����Υꥹ�Ȥ��ɲä������Ȼפ����⤷��ޤ���
X11 �Υޥ˥奢��ڡ����� /usr/X11R6/man ���֤���ޤ��������� MANPATH �Ķ��ѿ��ϸ�������ǥ��쥯�ȥ�Υꥹ�Ȥ�ޤ�Ǥ��ޤ���
</p>

<h2><a NAME="prefs">�Ķ�����</a></h2>
<p>��XDarwin�ץ�˥塼�ΡִĶ�����...�ץ�˥塼���ܤ��顤�����Ĥ��������Ԥ����Ȥ��Ǥ��ޤ��� 
�ֵ�ư���ץ����פ����Ƥϡ�XDarwin ��Ƶ�ư����ޤ�ͭ���Ȥʤ�ޤ���
¾�����ƤΥ��ץ��������Ƥϡ�ľ����ͭ���Ȥʤ�ޤ���
�ʲ������줾��Υ��ץ����ˤĤ����������ޤ�:</p>
<ul>
    <li>��������ܥ���: X11 �� Aqua ���ڤ��ؤ��뤿��Υ������Ȥ߹�碌���ѹ����뤿��ˡ��ܥ���򥯥�å����ơ������Ĥ��ν���������³�����̾�Υ����򲡤��ޤ���</li>
    <li>X11 �ǥ����ƥ�Υӡ��ײ�����Ѥ���: ����ξ�硤Mac OS X �Υӡ��ײ��� X11 �Υ٥�Ȥ��ƻ��Ѥ���ޤ������դξ��ʥǥե���ȡˡ�����ץ� �ȡ��󤬻Ȥ��ޤ���</li>
    <li>�ɥå��Υ�������Υ���å��� X11 �����: ����ξ�硤�ɥå���ɽ�����줿 XDarwin ��������Υ���å��� X11 �ؤ��ڤ��ؤ�����ǽ�Ȥʤ�ޤ���
    Mac OS X �ΥС������ˤ�äƤϡ��ɥå��Υ�������Υ���å��ǲ��̤��ڤ��ؤ���ȡ�Aqua ����ä����˥������뤬�ü����뤳�Ȥ�����ޤ���</li>
    <li>��ư���˥إ�פ�ɽ������: XDarwin �ε�ư���ˡ����ץ�å��� �����꡼���ɽ�����ޤ���</li>
    <li>�ǥ����ץ쥤�ֹ�:  XDarwin ���ǥ����ץ쥤�˳�����Ƥ� X �� Display Number ����ꤷ�ޤ��� 
    X11 ��ɽ������Ȥ���XDarwin �Ͼ�˥ᥤ��ǥ����ץ쥤������Ѥ����Ȥ���դ��Ʋ�������</li>
    <li>�����ޥåԥ�: �ǥե���ȤǤϡ�XDarwin �� Darwin �����ͥ뤫�饭���ޥåԥ󥰤���ɤ��ޤ���
        �ݡ����֥뵡��Ǥϡ����������ޥåԥ󥰤����ȤʤꡤX11 �ǥ����ܡ��ɤ�ư��ʤ��ʤ�ޤ���
       �֥ե����뤫����ɤ���פ����򤹤�ȡ�XDarwin �ϻ��ꤵ�줿�ե����뤫�饭���ޥåԥ󥰤���ɤ��ޤ���<br>
    �����������ޥåԥ󥰤� Japanese �����򤹤�ȡ��Զ�礬�����ޤ���USA �����򤷤���� ~/.Xmodmap ��Ŭ�Ѥ��Ʋ���������</li>
</ul>

<h2>
<a NAME="license">�饤����</a>
</h2>
XFree86 Project �ϡ���ͳ�˺����۲�ǽ�ʥХ��ʥ�ȥ����������ɤ��󶡤��뤳�Ȥ˥��ߥåȤ��Ƥ��ޤ���
�䤿�������Ѥ����ʥ饤���󥹤ϡ�����Ū�� MIT X11/X Consortium License �˴�Ť���ΤǤ���
�����ơ�����Ͻ����ޤ��Ϻ����ۤ���륽���������ɤޤ��ϥХ��ʥ�ˡ����� Copyright/�饤���󥹹𼨤����Τޤ޻Ĥ���뤳�Ȥ��׵᤹��ʳ��ξ��������ޤ���
���¿���ξ���ȡ������ɤΰ����򥫥С������ɲä� Copyright/�饤���󥹹𼨤Τ���ˡ�<A HREF="http://www.xfree86.org/legal/licence.html">XFree86 �� License �ڡ���</A>�򻲾Ȥ��Ʋ�������
<H3>
<A NAME="3"></A>
X  Consortium License</H3>
<p>Copyright (C) 1996 X Consortium</p>
<p>Permission is hereby granted, free of charge, to any person obtaining a 
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without
limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to
whom the Software is furnished to do so, subject to the following conditions:</p>
<p>The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.</p>
<p>THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT
SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.</p>
<p>Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization from
the X Consortium.</p>
<p>X Window System is a trademark of X Consortium, Inc.</p>
</body>
</html>
