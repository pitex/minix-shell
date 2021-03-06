(errata: w etapie 3 komenda do listowania zawartości bieżącego katalogu powinna się nazywać 'lls')


Etap 4. - przekierowanie we/wy i łącznie komend pipe'ami.

	1. Każdy progam uruchamiany z shella może mieć przekierowane wejście i wyjście. Na razie można ignorować przekierowanie komend shella. 

	- przekierowanie wejścia: jeżeli w struktrze command pole 'in_file_name' nie jest NULL'em to nowo uruchomiony program powinien mieć otwarty plik o nazwie wskazanej tym wskaźnikiem na standartowym wejściu (dekryptor 0).
	
	- przekierowanie wyjścia: jeżeli w struktrze command pole 'out_file_name' nie jest NULL'em to nowo uruchomiony program powinien mieć otwarty plik o nazwie wskazanej tym wskaźnikiem na standartowym wyjściu (dekryptor 1). Plik powinien zostać stworzony jeśli nie istnieje. Plik powinien zostać otwarty w trybie append jeśli pole 'append_mode' nie jest zerem, w przeciwnym przypadku plik powinien zostać wyczyszczony (opcja truncate).

	2. Jedna linia może zawierać wiele poleceń. W przypadku gdy linia zawiera więcej niż jedno polecenie można założyć że żadna z nich nie jest komendą shella. Należy wykonać wszystkie polecenia linii, każde w osobnym procesie potomnym shella. Kolejne polecenia powinny być połączone pipe'ami tak aby wyjście procesu realizującego polecenie k trafiało na wejście procesu polecenia k+1. Shell powinien zawiesić swoje działanie do momentu aż wszystkie procesy realizujące polecenia w danej linii się zakończą. Jeśli polecenie ma zdefiniowane prekierowanie(a) we/wy to mają one pierwszeństwo przed pipe'ami.

	3. Jedyną komendą shella która dopuszcza przekierowania jest 'lenv'. Należy zaimplementować przekierowania '>>' oraz '>'.

./mshell < test.e4.1.in > test.e4.1.out.m
sh < test.e4.1.in > test.e4.1.out.s
diff test.e4.1.out.m test.e4.1.out.s

./mshell < test.e4.2.in > test.e4.2.out.m
sh < test.e4.2.in > test.e4.2.out.s
diff test.e4.2.out.m test.e4.2.out.s

./mshell < test.e4.3.in > test.e4.3.out.m
sh < test.e4.3.in > test.e4.3.out.s
diff test.e4.3.out.m test.e4.3.out.s



Dodatkowe polecenia do testowanie p.2 (shell powinien się zawiesić na 10s)
> ls / | sleep 10 | ls | sort 

Testowanie p.3 
> lenv > AAA
> lenv >> AAA

