# pararatibum-compiler
Compilador para a linguagem PararaTibum, gera código C e Bytecode para JVM

Projeto desenvolvido como trabalho para a disciplina de compiladores, na FEMA/IMESA, 2015.
O bytecode gerado deve ser compilado com o Jasmin.

Exemplo de programas:

Verifica se um número é primo.

```
^menino e_primo(^menino n)
{
    ^parara (n = 0) {
        ^senta ^faz;
    }
    ^menino i := n - 1;
    ^menino primo := ^esqueca;
    ^pedindo_mais (i > 1) {
        ^parara ((n % i) = 0) {
            primo := ^faz;
        }
        i := i - 1;
    }
    ^senta primo;
}

@agora_eu_vou()
{
    ^novinha nome := "";
    ^mostrar("Qual seu nome? ");
    ^mexer_com(nome);
    ^mostrar("Seu nome e ");
    ^mostrar(nome);
    ^mostrar("\nBem vindo aos numeros primos\n");

    ^pedindo_mais (^esqueca) {

        
        ^mostrar("Entre com um numero: ");
        ^menino num := 0;
        ^mexer_com(num);
    
        ^parara (e_primo(num)) {
            ^mostrar(num);
            ^mostrar(" e um numero primo\n");
        } ^tibum {
            ^mostrar("nao e primo\n");
        }

    }
}
```

Calculo de fatoriais:
```
# =====================================================
# calcula o fatorial por iterações
# =====================================================
^menino fatorial(^menino joel)
{
    ^menino fat := 1;
    ^menino n := 1;
    ^pedindo_mais (n <= joel) {
        fat := fat * n;
        n := n + 1;
    }
    ^senta fat;
}


^menino fatorial_r(^menino joel)
{
    ^parara (joel < 2) {
        ^senta 1;
    } ^tibum {
        ^senta fatorial_r(joel - 1) * joel;
    }
}

@agora_eu_vou()
{
    ^menino num;
    ^mostrar("Digite o numero: ");
    ^mexer_com(num);
    ^menino fat := fatorial_r(num);
    ^mostrar("O fatorial de ");
    ^mostrar(num);
    ^mostrar(" e ");
    ^mostrar(fat);
}
```
