��       W     �     �      �   �  �   ,  �   5  �   N  �   7  	6   \  	n   _  	�   `  
+   u  
�   l     b  o   V  �   Y  )   ~  �   �     �  �   %  M     s     �     �   e  �     '     ;     R     o     �   $  �     �     �     �             #  .     R     m     u     �     �     �     �   H  �          .     M   !  k     �     �   (  �     �     �   #       5     O   $  o     �     �   #  �   B  �   2  4     g      {     �     �   *  �   *       /     O     _   #  m   #  �   &  �     �     �   ,  
     7     P   -  e     �     �     �     �     �     �               3  r  N  �  �   B  �   O  �   �     S  �   �  �   �  �        �  �   �  ?   �     o  �   �     �  �   �   j  �  !Y   5  #P   3  #�   %  #�   O  #�   �  $0     $�   6  %   B  %;   8  %~   5  %�   ]  %�   >  &K   @  &�     &�     &�   G  '	   [  'Q   E  '�     '�   =  (   %  (@   )  (f   )  (�      (�   �  (�   4  )s   O  )�   -  )�   J  *&   *  *q   6  *�   t  *�   :  +H   D  +�   b  +�   3  ,+   K  ,_   W  ,�   Q  -   3  -U   F  -�   �  -�   c  .X   &  .�   3  .�   9  /   5  /Q   E  /�   n  /�   O  0<   %  0�   '  0�   I  0�   I  1$   V  1n   0  1�   G  1�   i  2>   0  2�      2�   I  2�     3D   -  3_   -  3�   +  3�   4  3�   !  4   -  4>   -  4l   S  4�             >          	   0                 7      D   %   J             L      
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
 unexpected `,' unexpected `}' unknown command: `%c' unknown option to `s' unmatched `{' unterminated `s' command unterminated `y' command unterminated address regex Project-Id-Version: sed 4.2.0
Report-Msgid-Bugs-To: bug-gnu-utils@gnu.org
POT-Creation-Date: 2009-04-30 10:58+0200
PO-Revision-Date: 2008-01-17 23:34+0200
Last-Translator: Pavel Maryanov <acid_jack@ukr.net>
Language-Team: Russian <gnu@mx.ru>
MIME-Version: 1.0
Content-Type: text/plain; charset=utf-8
Content-Transfer-Encoding: 8bit
Plural-Forms: nplurals=2; plural=n>1;
 
Если опция -e, --expression, -f, или --file не указана, тогда первый
неопциональный аргумент берется как скрипт sed для интерпретации. Все
оставшиеся аргументы являются именами входных файлов; если входные
файлы не указаны, тогда читается стантартный ввод.

       --help     вывод этой справки и выход
       --version  вывод информации о версии и выход
   --follow-symlinks
                 переходить по символьным ссылкам при обработке на месте
   --posix
                 отключение всех расширений GNU.
   -R, --regexp-perl
                 использование в скрипте синтаксиса регулярных выражений Perl 5.
   -b, --binary
                 открывать файлы в бинарном режиме (CR+LF не обрабатываются)
   -e script, --expression=script
                 добавление скрипта в исполняемые команды
   -f script-file, --file=script-file
                 добавление содержимого файла-скрипта в исполняемые команды
   -i[СУФФИКС], --in-place[=СУФФИКС]
                 редактирование файлов на месте (создает копию, если указано расширение)
   -l N, --line-length=N
                 указание желаемой длины переносимой строки для команды `l'
   -n, --quiet, --silent
                 не выводить автоматически промежутки
   -r, --regexp-extended
                 использование в скрипте расширенных регулярных выражений.
   -s, --separate
                 допущение, что файлы разделены, а не в виде одного
                 длинного непрерывного потока.
   -u, --unbuffered
                 загрузка минимального объема данных из входных файлов
                 и более частый сброс на диск выходных буферов
 %s
Это свободное программное обеспечение; условия его копирования смотрите в
исходных текстах. Не предоставляется НИКАКОЙ гарантии; даже гарантии
ПРИГОДНОСТИ ДЛЯ ПРОДАЖИ или ПРИМЕНИМОСТИ ДЛЯ КОНКРЕТНОЙ ЦЕЛИ, в той мере,
в которой это может быть допущено законодательством.
 %s: -e выражение #%lu, символ %lu: %s
 %s: невозможно прочитать %s: %s
 %s: файл %s строка %lu: %s
 `:' не допускает указания каких-либо адресов Отчеты об ошибках отправляйте по адресу: <%s>.
Убедитесь, что включили где-либо в поле ``Тема:'' слово ``%s''.
 GNU sed версия %s
 Недопустимая обратная ссылка Недопустимое имя для класса символа Недопустимый символ сравнения Недопустимое содержимое в \{\} Недопустимое предшествующее регулярное выражение Недопустимое окончание диапазона Недопустимое регулярное выражение Память исчерпана Нет соотвествия Нет предыдущего регулярного выражения Преждевременное окончание регулярного выражения Регулярное выражение слишком большое Успешно Завершающая обратная косая черта Непарный символ ( or \( Непарный символ ) или \) Непарный символ [ или [^ Непарный символ \{ Использование: %s [ОПЦИЯ]... {только-скрипт-если-нет-другого-скрипта} [входной-файл]...

 команда `e' не поддерживается `}' не допускает указания каких-либо адресов основан на GNU sed версии %s

 невозможно найти метку для перехода к `%s' невозможно удалить %s: %s невозможно переименовать %s: %s невозможно указать модификаторы в пустом регулярном выражении невозможно выполнить stat для %s: %s команда использует только один адрес комментарии не допускают указания каких-либо адресов невозможно прикрепить к %s: %s невозможно редактировать %s: это терминал невозможно редактировать %s: это не обычный файл невозможно перейти по символьной ссылке %s: %s невозможно открыть файл %s: %s невозможно открыть временный файл %s: %s невозможно записать %d элемент в %s: %s невозможно записать %d элементов в %s: %s символ-разделитель не является однобайтовым символом ошибка в подпроцессе ожидалась \ после `a', `c' или `i' ожидалась более новая версия sed лишние символы после команды недопустимая ссылка \%d на RHS команды `s' использование +N или ~N в качестве первого адреса недопустимо недопустимое использование строки адреса 0 отсутствует команда несколько символов `!' несколько модификаторов `g' с командой `s' несколько модификаторов `p' с командой `s' несколько числовых модификаторов с командой `s' отсутствуют входные файлы нет предыдущего регулярного выражения числовой модификатор для команды `s' не может быть нулевым опция `e' не поддерживается ошибка чтения %s: %s строки для команды `y' имеют разную длину super-sed версия %s
 непредвиденный символ `,' непредвиденный символ `}' неизвестная команда: `%c' неизвестный модификатор к `s' непарный символ `{' незавершенная команда `s' незавершенная команда `y' незавершенное адресное регулярное выражение 