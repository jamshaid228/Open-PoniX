��       P     �   k        �   �  �   ,  �   5  �   7  '   \  _   `  �   u  	   l  	�   b  
    Y  
c   ~  
�   �  <   �  �   %  �     �     �     �   e  �     a     u     �     �     �   $  �               /     @     I   #  h     �     �     �     �     �     �     �   H       N     h     �   !  �     �     �   (  �        #  8     \   $  |     �   #  �   B  �   2  "     U      i     �     �   *  �   *  �          =     M   #  [   #     &  �     �   ,  �          /   -  D     r     �     �     �     �     �     �     �       �  -   �  �   (  �   .  �   /  �   N     a  k     �   o  M   K  �   J  	   K  T   �  �  �  "   !  �     �             h  5     �     �     �     �     �     �               !     6     C     `     u     �     �     �     �     �     �   I  �   '  5     ]     w   *  �     �     �   &  �   #     "  4   $  W   4  |      �   &  �   ,  �   (   &      O   $   b      �   $   �   $   �   (   �     !     !(     !=   $  !M   $  !r   #  !�     !�   1  !�   )  "
     "4   )  "I     "s     "�     "�     "�     "�     "�     "�     #     #6                      (                      4      
                       M   	   6           8   O   I   ,   =            1   5      :      A                F   P            '   2       <   /          !                     +   )       $   7   %   &   -   >       3   D   L           E      0   9                      B       *   C   ?   N   K      .      ;       G   "   J       #       @   H     
If no -e, --expression, -f, or --file option is given, then the first
non-option argument is taken as the sed script to interpret.  All
remaining arguments are names of input files; if no input files are
specified, then the standard input is read.

       --help     display this help and exit
       --version  output version information and exit
   --posix
                 disable all GNU extensions.
   -R, --regexp-perl
                 use Perl 5's regular expressions syntax in the script.
   -e script, --expression=script
                 add the script to the commands to be executed
   -f script-file, --file=script-file
                 add the contents of script-file to the commands to be executed
   -i[SUFFIX], --in-place[=SUFFIX]
                 edit files in place (makes backup if extension supplied)
   -l N, --line-length=N
                 specify the desired line-wrap length for the `l' command
   -r, --regexp-extended
                 use extended regular expressions in the script.
   -s, --separate
                 consider files as separate rather than as a single continuous
                 long stream.
   -u, --unbuffered
                 load minimal amounts of data from the input files and flush
                 the output buffers more often
 %s
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE,
to the extent permitted by law.
 %s: -e expression #%lu, char %lu: %s
 %s: can't read %s: %s
 %s: file %s line %lu: %s
 : doesn't want any addresses E-mail bug reports to: <%s>.
Be sure to include the word ``%s'' somewhere in the ``Subject:'' field.
 GNU sed version %s
 Invalid back reference Invalid character class name Invalid collation character Invalid content of \{\} Invalid preceding regular expression Invalid range end Invalid regular expression Memory exhausted No match No previous regular expression Premature end of regular expression Regular expression too big Success Trailing backslash Unmatched ( or \( Unmatched ) or \) Unmatched [ or [^ Unmatched \{ Usage: %s [OPTION]... {script-only-if-no-other-script} [input-file]...

 `e' command not supported `}' doesn't want any addresses based on GNU sed version %s

 can't find label for jump to `%s' cannot remove %s: %s cannot rename %s: %s cannot specify modifiers on empty regexp command only uses one address comments don't accept any addresses couldn't edit %s: is a terminal couldn't edit %s: not a regular file couldn't open file %s: %s couldn't open temporary file %s: %s couldn't write %d item to %s: %s couldn't write %d items to %s: %s delimiter character is not a single-byte character error in subprocess expected \ after `a', `c' or `i' expected newer version of sed extra characters after command invalid reference \%d on `s' command's RHS invalid usage of +N or ~N as first address invalid usage of line address 0 missing command multiple `!'s multiple `g' options to `s' command multiple `p' options to `s' command multiple number options to `s' command no previous regular expression number option to `s' command may not be zero option `e' not supported read error on %s: %s strings for `y' command are different lengths super-sed version %s
 unexpected `,' unexpected `}' unknown command: `%c' unknown option to `s' unmatched `{' unterminated `s' command unterminated `y' command unterminated address regex Project-Id-Version: GNU sed 4.1.1
Report-Msgid-Bugs-To: bug-gnu-utils@gnu.org
POT-Creation-Date: 2009-04-30 10:58+0200
PO-Revision-Date: 2005-02-01 21:02+0900
Last-Translator: IIDA Yosiaki <iida@gnu.org>
Language-Team: Japanese <translation-team-ja@lists.sourceforge.net>
MIME-Version: 1.0
Content-Type: text/plain; charset=EUC-JP
Content-Transfer-Encoding: 8bit
Plural-Forms: nplurals=1; plural=0;
 
-e��--expression��-f��--file���ץ����Τɤ��ʤ��ȡ����ץ����ʳ���
�ǽ�ΰ�����sed������ץȤȤ��Ʋ�ᤷ�ޤ����Ĥ�ΰ��������������ϥե�
����̾�Ȥʤ�ޤ������ϥե�����λ��꤬�ʤ��ȡ�ɸ�����Ϥ��ɤ߹��ߤޤ���

       --help     ����������ɽ�����ƽ�λ
       --version  �С����������ɽ�����ƽ�λ
   --posix
                 GNU��ĥ�������ػߡ�
   -R, --regexp-perl
                 ������ץȤ�Perl 5������ɽ����ʸ��Ȥ���
   -e ������ץ�, --expression=������ץ�
                 �¹Ԥ��륳�ޥ�ɤȤ��ƥ�����ץȤ��ɲ�
   -f ������ץȡ��ե�����, --file=������ץȡ��ե�����
                 �¹Ԥ��륳�ޥ�ɤȤ��ƥ�����ץȡ��ե���������Ƥ��ɲ�
   -i[������], --in-place[=������]
                 �ե�����򤽤ξ���Խ� (��ĥ�Ҥ�����С��Хå����åפ����)
   -l N, --line-length=N
                 ��l�ץ��ޥ���Ѥι����֤�Ĺ�����
   -r, --regexp-extended
                 ������ץȤǳ�ĥ����ɽ������ѡ�
   -s, --separate
                 �ե�������Ϣ�����Ϥˤ������̡��˽�����
   -u, --unbuffered
                 ���ϥե����뤫��˾��Υǡ���������ߡ�
                 ���礯���礯���ϥХåե������ݽФ�
 %s
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE,
to the extent permitted by law.

����: ���˽��פ�ʸ�ϤΤ��ᡢ��ʸ��Ĥ��Ƥ��ޤ���
  -- ������
����ϥե꡼�����եȥ������Ǥ���ʣ���ξ��˴ؤ��Ƥϡ�����������������
�����ݾڤϰ��ڤ���ޤ��󡣱�����Ū��ˡ������줿�ϰϤǤ�������Ū�Τ���
��Ŭ�����⤢��ޤ���
 %s: -e ɽ�� #%lu, ʸ���� %lu: %s
 %s: %s���ɤ߹���ޤ���: %s
 %s: �ե����� %s %lu��: %s
 :�˥��ɥ쥹�����פǤ� �Żҥ᡼��ˤ��Х����ΰ���: <%s>
���κݡ���Subject:�ɥե�����ɤΤɤ����ˡ�%s�ɤ�����Ƥ���������
 GNU sed %s��

 ̵���ʵջ��� ̵����ʸ�����饹̾ ̵���ʹ���ʸ�� ̵����\{\}������" ̵�����������ɽ�� ̵�����ϰϤν�ü ̵��������ɽ�� ���꡼��­��ޤ��� �ȹ礷�ޤ��� ľ��������ɽ����������ޤ��� ��᤮������ɽ����ü �礭�᤮������ɽ�� ���� ��³�εե���å��� (��\(����礤�ޤ��� �����ʤ�)��\) [��[^����礤�ޤ��� \{����礤�ޤ��� ����ˡ: %s [���ץ����]... {������ץ�(¾�ˤʤ����)} [���ϥե�����]...

 ��e�ץ��ޥ�ɤϡ����ݡ��Ȥ���Ƥ��ޤ��� ��}�פ˥��ɥ쥹�����פǤ� ����GNU sed %s��

 ��%s�פؤΥ����פΥ�٥뤬���Ĥ���ޤ��� %s�����Ǥ��ޤ���: %s %s��̾�����ѹ��Ǥ��ޤ���: %s �����Ҥϡ���������ɽ���˻���Ǥ��ޤ��� ���ޥ�ɤϥ��ɥ쥹��1�Ĥ����Ȥ��ޤ� �����Ȥϥ��ɥ쥹������դ��ޤ��� %s�ϡ��Խ��Ǥ��ޤ���Ǥ���: ü���Ǥ� %s�ϡ��Խ��Ǥ��ޤ���Ǥ���: �̾�ե�����Ǥ���ޤ��� �ե�����%s�򳫤��ޤ���Ǥ���: %s ����ե�����򳫤��ޤ���Ǥ���: %s: %s %d�ĤΥ����ƥ��%s�ؽ񤭹���ޤ���Ǥ���: %s ���ڤ�ʸ������ñ��Х���ʸ���Ǥ���ޤ��� �ҥץ������Υ��顼 \����a�ס�c�ס�i�פθ��ͽ������ޤ� sed�ο��Ǥ�����Ǥ� ���ޥ�ɤθ����;�פ�ʸ��������ޤ� ��s�ץ��ޥ�ɤα�¦��̵����\%d�λ��� �ǽ�Υ��ɥ쥹�ؤ�+N��~N�λ����̵���Ǥ� ̵���ʹԥ��ɥ쥹0�λ���ˡ ���ޥ�ɤ�­��ޤ��� ʣ���Ρ�!�פǤ� ��s�ץ��ޥ�ɤ�ʣ���Ρ�g�ץ��ץ���� ��s�ץ��ޥ�ɤ�ʣ���Ρ�p�ץ��ץ���� ��s�ץ��ޥ�ɤ�ʣ���ο��ͥ��ץ���� ľ��������ɽ����������ޤ��� ��s�ץ��ޥ�ɤؤο��ͥ��ץ��������ǤϤ����ޤ��� ��e�ץ��ץ����ϡ����ݡ��Ȥ���Ƥ��ޤ��� %s���ɹ��ߥ��顼: %s ��y�ץ��ޥ�ɤؤ�ʸ�����Ĺ�������㤤�ޤ� super-sed %s��
 ͽ�����̡�,�פǤ� ͽ�����̡�}�פǤ� ̤�ΤΥ��ޥ�ɤǤ�: ��%c�� ��s�פؤΥ��ץ����̤�ΤǤ� �����ʤ���{�פǤ� ��s�ץ��ޥ�ɤ���λ���Ƥ��ޤ��� ��y�ץ��ޥ�ɤ���λ���Ƥ��ޤ��� ���ɥ쥹regex����λ���Ƥ��ޤ��� 