#!/bin/bash

# Script para migrar o projeto para o repositório couplesfinance

echo "🚀 Migrando projeto para couplesfinance..."
echo ""

# Verifica se gh CLI está disponível
if command -v gh &> /dev/null; then
    echo "📦 GitHub CLI detectado. Criando repositório..."

    # Cria o repositório
    gh repo create ThGalvani/couplesfinance \
        --public \
        --description "💰 FinançaCasal - Gestão financeira compartilhada para casais" \
        --clone=false

    if [ $? -eq 0 ]; then
        echo "✅ Repositório criado com sucesso!"
    else
        echo "⚠️  Repositório pode já existir ou houve um erro. Continuando..."
    fi
else
    echo "⚠️  GitHub CLI (gh) não encontrado."
    echo "📋 Por favor, crie o repositório manualmente:"
    echo "   https://github.com/new"
    echo "   Nome: couplesfinance"
    echo ""
    read -p "Pressione ENTER após criar o repositório no GitHub..."
fi

echo ""
echo "📤 Fazendo push para couplesfinance..."

# Configura remote
git remote set-url origin http://local_proxy@127.0.0.1:43450/git/ThGalvani/couplesfinance

# Faz push
git push -u origin claude/couples-finance-app-5MmPs

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Push realizado com sucesso!"
    echo "🔗 Repositório: https://github.com/ThGalvani/couplesfinance"
    echo "🌿 Branch: claude/couples-finance-app-5MmPs"
else
    echo ""
    echo "❌ Erro ao fazer push. Verifique se o repositório foi criado corretamente."
    echo "   URL esperada: https://github.com/ThGalvani/couplesfinance"
fi
