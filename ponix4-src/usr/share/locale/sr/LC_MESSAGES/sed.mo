��       R     �   m  <      �   �  �   ,  �   5     7  O   \  �   `  �   u  	E   l  	�   b  
(   V  
�   Y  
�   ~  <   �  �   �  K   %       ,     C     ]   e  z     �     �          (     D   $  \     �     �     �     �     �   #  �          &     .     A     S     e     w   H  �     �     �        !  $     F     [   (  p     �   #  �     �   $  �         #  :   B  ^   2  �     �      �     	     '   *  F   *  q     �     �     �   #  �   #  �   &  "     I     X   ,  w     �     �   -  �                %     4     J     `     n     �     �  �  �  �  �   J  5   =  �   L  �   �     n  �   �     �  �   s  c   |  �   z  T   �  �   �  ]    5   )  :   '  d   '  �   /  �   �  �      �   8   �   5   �   2  !   +  !C   G  !o   *  !�   2  !�   #  "     "9   =  "S   7  "�   5  "�   
  "�   /  #
     #:     #Y     #x     #�   �  #�   0  $9   5  $j   6  $�   F  $�   %  %   .  %D   S  %s   >  %�   F  &   6  &M   G  &�   6  &�   K  '   �  'O   =  '�   $  (;   ?  (`   5  (�   3  (�   a  )
   K  )l   =  )�   !  )�     *   9  *)   9  *c   D  *�   (  *�   =  +   T  +I   .  +�   +  +�   M  +�     ,G     ,d     ,�   +  ,�   ,  ,�     ,�   +  -   +  -@   6  -l            $           E       R       "          !       =   F       M   B      5         ?      4   3                .               O      @         ;           2   (   :       9   %           <   	   /       C       Q   '   ,   L                 N                
          #   6   +   0       1       7   G         D   I   *      -   >   K         P   8       &          J   )                 H      A 
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
   -n, --quiet, --silent
                 suppress automatic printing of pattern space
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

 can't find label for jump to `%s' cannot remove %s: %s cannot rename %s: %s cannot specify modifiers on empty regexp command only uses one address comments don't accept any addresses couldn't edit %s: is a terminal couldn't edit %s: not a regular file couldn't open file %s: %s couldn't open temporary file %s: %s couldn't write %d item to %s: %s couldn't write %d items to %s: %s delimiter character is not a single-byte character error in subprocess expected \ after `a', `c' or `i' expected newer version of sed extra characters after command invalid reference \%d on `s' command's RHS invalid usage of +N or ~N as first address invalid usage of line address 0 missing command multiple `!'s multiple `g' options to `s' command multiple `p' options to `s' command multiple number options to `s' command no input files no previous regular expression number option to `s' command may not be zero option `e' not supported read error on %s: %s strings for `y' command are different lengths super-sed version %s
 unexpected `,' unexpected `}' unknown command: `%c' unknown option to `s' unmatched `{' unterminated `s' command unterminated `y' command unterminated address regex Project-Id-Version: sed 4.1.5
Report-Msgid-Bugs-To: bug-gnu-utils@gnu.org
POT-Creation-Date: 2009-04-30 10:58+0200
PO-Revision-Date: 2006-12-07 11:42-0500
Last-Translator: Aleksandar Jelenak <jelenak@verizon.net>
Language-Team: Serbian <gnu@prevod.org>
MIME-Version: 1.0
Content-Type: text/plain; charset=UTF-8
Content-Transfer-Encoding: 8bit
Plural-Forms: nplurals=3;    plural=n%10==1 && n%100!=11 ? 0 :  (n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2);
 
Уколико „-e“, „--expression“, „-f“, или „--file“ опција није задата онда
се први ванопциони аргумент узима као сед спис за тумачење. Сви преостали
аргументи су имена улазних датотека; ако они нису задати, онда се чита са
стандардног улаза.

       --help     прикажи ово објашњење и заврши
       --version  испиши верзију и заврши
   --posix
                 онемогући све ГНУ додатке.
   -R, --regexp-perl
                 користи у спису синтаксу Перла 5 за регуларне изразе.
   -e спис, --expression=спис
                 додај спис извршним наредбама
   -f списотека, --file=списотека
                 додај садржај списотеке извршним наредбама
   -i[СУФИКС], --in-place[=СУФИКС]
                 уређуј датотеке у месту (прави резервну копију ако је дат
                 наставак)
   -l N, --line-length=N
                 задај жељену ширину реда за наредбу „l“
   -n, --quiet, --silent
                 обустави аутоматски испис простора образаца
   -r, --regexp-extended
                 користи проширене регуларне изразе у спису.
   -s, --separate
                 посматрај датотеке одвојено а не као један непрекидан ток
   -u, --unbuffered
                 учитавај минималне количине података из улазних датотека и
                 чешће празни излазне бафере
 %s
Ово је слободан софтвер; видети изворни кôд за услове умножавања. БЕЗ икакве
гаранције; чак ни о ТРЖИШНОСТИ или ИСПУЊАВАЊУ ОДРЕЂЕНЕ ПОТРЕБЕ.
 %s: -e израз #%lu, знак %lu: %s
 %s: не може читати  %s: %s
 %s: датотека %s ред %lu: %s
 : не захтева икакве адресе Епошта за пријаву грешака: <%s>.
Постарајте се да укључите реч „%s“ негде у „Subject:“ пољу.
 ГНУ sed верзија %s
 Неисправна повратна референца Неисправно име класе знакова Неисправни знак прикупљања Неисправни садржај у \{\} Неисправан претходећи регуларни израз Неисправни крај опсега Неисправни регуларни израз Меморија исцрпљена Без поклапања Без претходног регуларног израза Преран крај регуларног израза Регуларни израз сувише велик Успех Пратећа обрнута коса црта Неспарено ( или \( Неспарено ) или \) Неспарено [ или ^[ Неспарено \{ Употреба: %s [ОПЦИЈА]... {скрипт-само-ако-нема-другог-скрипта} [улаз-датотека]...

 наредба „e“ није подржана „}“ не захтева икакве адресе засновано на ГНУ sed верзија %s

 не могу да нађем ознаку за скок на „%s“ не може уклонити %s: %s не може променити име %s: %s не може навести измењивач празном рег. изразу наредба користи само једну адресу коментари не прихватају икакве адресе не могу уредити %s: терминал је не може уредити %s: није обична датотека не може отворити датотеку %s: %s не могу отворити привремену датотеку %s: %s неуспешан запис %d ставке на %s: %s неуспешан запис %d ставке на %s: %s неуспешан запис %d ставки на %s: %s раздвојник није једнобајтни знак грешка у потпроцесу очекивано \ после „a“, „c“ или „i“ очекивана новија верзија sed-а вишак знакова после наредбе неисправна референца \%d на десној страни наредбе „s“ неважећа употреба +N или ~N као прве адресе неправилна употреба адресе реда 0 недостаје наредба више „!“ више „g“ опција за „s“ наредбу више „p“ опција за „s“ наредбу више бројчаних опција за „s“ наредбу нема улазних датотека без претходног регуларног израза бројчана опција наредбе „s“ не може бити нула опција „e“ није подржана грешка учитавања на %s: %s ниске за команду „y“ су различитих дужина super-sed верзија %s
 неочекиван „,“ неочекивана „}“ непозната наредба: „%c“ непозната опција за „s“ неспарена „{“ незавршена наредба „s“ незавршена наредба „y“ незавршена адреса рег. израза 