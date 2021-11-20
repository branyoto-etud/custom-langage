make all

rm result.txt

cd Jeu_de_tests/correct/
fichiers1=( $(ls) )
cd ../../
for i in "${fichiers1[@]}"
do
    echo "$i" >> result.txt
    ./bin/compil < Jeu_de_tests/correct/"$i" >> result.txt
    mv tmp.asm "${i/%tpc/asm}"
done


cd Jeu_de_tests/warning/
fichiers1=( $(ls) )
cd ../../
for i in "${fichiers1[@]}"
do
    echo "$i" >> result.txt
    ./bin/compil < Jeu_de_tests/warning/"$i" >> result.txt
    mv tmp.asm "${i/%tpc/asm}"
done


cd Jeu_de_tests/error/
fichiers2=( $(ls) )
cd ../../
for i in "${fichiers2[@]}"
do
    echo "$i" >> result.txt
    ./bin/compil < Jeu_de_tests/error/"$i" >> result.txt
done
