��       W     �     �      �   �  �   ,  �   5  �   N  �   7  	6   \  	n   _  	�   `  
+   u  
�   l     b  o   V  �   Y  )   ~  �   �     �  �   %  M     s     �     �   e  �     '     ;     R     o     �   $  �     �     �     �             #  .     R     m     u     �     �     �     �   H  �          .     M   !  k     �     �   (  �     �     �   #       5     O   $  o     �     �   #  �   B  �   2  4     g      {     �     �   *  �   *       /     O     _   #  m   #  �   &  �     �     �   ,  
     7     P   -  e     �     �     �     �     �     �               3  �  N    �   -     0  =   [  n   ;  �   Y     _  `   c  �   y  $   �  �   c  D   Z  �   ^     _  b   �  �   �  S   "  $     G     b     �   �  �      ;      N      h      �      �   *   �      �      �      !     !7      !K   (  !l     !�     !�     !�     !�     !�     !�     "   L  "     "g     "�      "�   +  "�     "�     #   9  #%   &  #_   '  #�   !  #�     #�   %  #�   ,  $   ,  $B     $o   (  $�   N  $�   )  %     %0   (  %B   +  %k     %�   0  %�   .  %�   !  &     &2     &G   $  &T   $  &y   %  &�     &�      &�   -  &�     '(     'F   ;  'Y     '�     '�     '�     '�      '�     (     (     (-   +  (G             >          	   0                 7      D   %   J             L      
   .   F   M           @   G       ;   =                      S   9   5          ,   C                 B   (   O       -                          :                      K   4               N   /              +       V                '             Q                     A   8       &   P   W   I   2   H      $              "   #   6   <   U       R      )   *       E   ?      3   T   !   1       
If no -e, --expression, -f, or --file option is given, then the first
non-option argument is taken as the sed script to interpret.  All
remaining arguments are names of input files; if no input files are
specified, then the standard input is read.

       --help     display this help and exit
       --version  output version information and exit
   --follow-symlinks
                 follow symlinks when processing in place
   --posix
                 disable all GNU extensions.
   -R, --regexp-perl
                 use Perl 5's regular expressions syntax in the script.
   -b, --binary
                 open files in binary mode (CR+LFs are not processed specially)
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

 can't find label for jump to `%s' cannot remove %s: %s cannot rename %s: %s cannot specify modifiers on empty regexp cannot stat %s: %s command only uses one address comments don't accept any addresses couldn't attach to %s: %s couldn't edit %s: is a terminal couldn't edit %s: not a regular file couldn't follow symlink %s: %s couldn't open file %s: %s couldn't open temporary file %s: %s couldn't write %d item to %s: %s couldn't write %d items to %s: %s delimiter character is not a single-byte character error in subprocess expected \ after `a', `c' or `i' expected newer version of sed extra characters after command invalid reference \%d on `s' command's RHS invalid usage of +N or ~N as first address invalid usage of line address 0 missing command multiple `!'s multiple `g' options to `s' command multiple `p' options to `s' command multiple number options to `s' command no input files no previous regular expression number option to `s' command may not be zero option `e' not supported read error on %s: %s strings for `y' command are different lengths super-sed version %s
 unexpected `,' unexpected `}' unknown command: `%c' unknown option to `s' unmatched `{' unterminated `s' command unterminated `y' command unterminated address regex Project-Id-Version: sed-4.2.0
Report-Msgid-Bugs-To: bug-gnu-utils@gnu.org
POT-Creation-Date: 2009-04-30 10:58+0200
PO-Revision-Date: 2008-01-17 22:49+0100
Last-Translator: Benno Schulenberg <benno@vertaalt.nl>
Language-Team: Dutch <vertaling@vrijschrift.org>
MIME-Version: 1.0
Content-Type: text/plain; charset=UTF-8
Content-Transfer-Encoding: 8bit
Plural-Forms: nplurals=2; plural=(n != 1);
X-Generator: KBabel 1.11.4
 
Als er geen '-e', '--expression', '-f' of '--file' gegeven is, wordt het
eerste niet-optie argument als het te interpreteren 'sed'-script genomen.
Alle overblijvende argumenten zijn namen van invoerbestanden; als er geen
invoerbestanden gegeven zijn, wordt standaardinvoer gelezen.

   --help     deze hulptekst tonen en stoppen
   --version  versie-informatie tonen en stoppen
   --follow-symlinks
             symbolische koppelingen volgen (bij bewerking ter plekke)
   --posix
             alle GNU-uitbreidingen uitschakelen
   -R, --regexp-perl
             reguliere expressies van Perl-5 gebruiken in het script
   -b, --binary
             bestanden openen in binaire modus (regeleinden zijn niet speciaal)
   -e SCRIPT, --expression=SCRIPT
             dit SCRIPT toevoegen aan de uit te voeren opdrachten
   -f SCRIPTBESTAND, --file=SCRIPTBESTAND
             inhoud van SCRIPTBESTAND toevoegen aan de uit te voeren opdrachten
   -i[ACHTERVOEGSEL], --in-place[=ACHTERVOEGSEL]
             bestanden ter plekke bewerken
             (en een reservekopie maken als een ACHTERVOEGSEL gegeven is)
   -l AANTAL, --line-length=AANTAL
             de gewenste regelafbreeklengte voor de 'l'-opdracht
   -n, --quiet, --silent
             automatische weergave van patroonruimte onderdrukken
   -r, --regexp-extended
             uitgebreide reguliere expressies gebruiken in het script
   -s, --separate
             bestanden als losstaand beschouwen, niet als één enkele stroom
   -u, --unbuffered
             minimale hoeveelheden gegevens laden uit de invoerbestanden,
             en de uitvoerbuffers vaker leegmaken

 %s
Dit is vrije software; zie de programmatekst voor de kopieervoorwaarden.
Er is GEEN ENKELE garantie, zelfs niet voor VERHANDELBAARHEID of
GESCHIKTHEID VOOR EEN BEPAALD DOEL, voorzover de wet dit toestaat.
 %s: expressie #%lu, teken %lu: %s
 %s: kan %s niet lezen: %s
 %s: bestand %s, regel %lu: %s
 ':' accepteert geen adressen Rapporteer gebreken in het programma aan <%s>
met het woord "%s" ergens in de Onderwerp-regel;
meld fouten in de vertaling aan <vertaling@vrijschrift.org>.
 GNU sed versie %s
 Ongeldige terugverwijzing Ongeldige tekenklassenaam Ongeldig samengesteld teken Ongeldige inhoud van \{\} Ongeldige voorafgaande reguliere expressie Ongeldig bereikeinde Ongeldige reguliere expressie Onvoldoende geheugen beschikbaar Geen overeenkomsten Geen eerdere reguliere expressie Voortijdig einde van reguliere expressie Reguliere expressie is te groot Gelukt Backslash aan het eind Ongepaarde ( of \( Ongepaarde ) of \) Ongepaarde [ of [^ Ongepaarde \{ Gebruik:  %s [OPTIE]... {SCRIPT_als_verder_geen_script} [INVOERBESTAND]...

 'e'-opdracht is niet mogelijk '}' accepteert geen adressen gebaseerd op GNU sed versie %s

 kan label voor sprong naar '%s' niet vinden kan %s niet verwijderen: %s kan %s niet hernoemen: %s bij een lege reguliere expressie passen geen veranderaars kan de status van %s niet opvragen: %s opdracht accepteert slechts één adres opmerkingen accepteren geen adres kan niet aan %s aanhechten: %s kan %s niet bewerken: is een terminal kan %s niet bewerken: is geen gewoon bestand kan symbolische koppeling %s niet volgen: %s kan bestand %s niet openen: %s kan tijdelijk bestand %s niet openen: %s kan %d item niet naar %s schrijven: %s kan %d items niet naar %s schrijven: %s scheidingsteken is niet één enkele byte fout in subproces een '\' werd verwacht na 'a', 'c' of 'i' een nieuwere versie van 'sed' werd verwacht extra tekens na opdracht ongeldige verwijzing \%d rechts van 's'-opdracht ongeldig gebruik van +N of ~N als eerste adres ongeldig gebruik van regeladres 0 ontbrekende opdracht meerdere '!' meerdere 'g'-opties bij 's'-opdracht meerdere 'p'-opties bij 's'-opdracht meerdere getalopties bij 's'-opdracht geen invoerbestanden geen eerdere reguliere expressie getaloptie bij 's'-opdracht mag niet nul zijn 'e'-opdracht is niet mogelijk leesfout op %s: %s tekenreeksen bij 'y'-opdracht zijn van verschillende lengte super-sed versie %s
 onverwachte ',' onverwachte '}' onbekende opdracht: '%c' onbekende optie bij 's'-opdracht ongepaarde '{' onafgemaakte 's'-opdracht onafgemaakte 'y'-opdracht onafgemaakte reguliere expressie voor adres 