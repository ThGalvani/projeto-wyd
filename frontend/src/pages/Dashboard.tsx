import MainLayout from '@/components/layout/MainLayout'
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card'
import { TrendingUp, TrendingDown, Wallet, Receipt } from 'lucide-react'

export default function Dashboard() {
  const stats = [
    {
      title: 'Receita Mensal',
      value: 'R$ 0,00',
      icon: TrendingUp,
      color: 'text-success',
      bgColor: 'bg-success-50',
    },
    {
      title: 'Despesas Mensais',
      value: 'R$ 0,00',
      icon: TrendingDown,
      color: 'text-danger',
      bgColor: 'bg-danger-50',
    },
    {
      title: 'Saldo',
      value: 'R$ 0,00',
      icon: Wallet,
      color: 'text-primary',
      bgColor: 'bg-primary-50',
    },
    {
      title: 'Transações',
      value: '0',
      icon: Receipt,
      color: 'text-warning',
      bgColor: 'bg-warning-50',
    },
  ]

  return (
    <MainLayout>
      <div className="space-y-6">
        <div>
          <h1 className="text-3xl font-bold text-gray-900">Dashboard</h1>
          <p className="mt-2 text-gray-600">
            Acompanhe suas finanças em tempo real
          </p>
        </div>

        {/* Stats Grid */}
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-6">
          {stats.map((stat) => {
            const Icon = stat.icon
            return (
              <Card key={stat.title}>
                <CardContent className="p-6">
                  <div className="flex items-center justify-between">
                    <div>
                      <p className="text-sm font-medium text-gray-500">
                        {stat.title}
                      </p>
                      <p className={`mt-2 text-3xl font-bold ${stat.color}`}>
                        {stat.value}
                      </p>
                    </div>
                    <div className={`p-3 rounded-full ${stat.bgColor}`}>
                      <Icon className={`h-6 w-6 ${stat.color}`} />
                    </div>
                  </div>
                </CardContent>
              </Card>
            )
          })}
        </div>

        {/* Placeholder Cards */}
        <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
          <Card>
            <CardHeader>
              <CardTitle>Transações Recentes</CardTitle>
            </CardHeader>
            <CardContent>
              <p className="text-gray-500">Nenhuma transação registrada ainda.</p>
            </CardContent>
          </Card>

          <Card>
            <CardHeader>
              <CardTitle>Despesas por Categoria</CardTitle>
            </CardHeader>
            <CardContent>
              <p className="text-gray-500">Adicione transações para ver o gráfico.</p>
            </CardContent>
          </Card>
        </div>

        <Card>
          <CardHeader>
            <CardTitle>Status do Desenvolvimento</CardTitle>
          </CardHeader>
          <CardContent>
            <div className="space-y-4">
              <div className="flex items-center gap-3">
                <div className="h-2 w-2 rounded-full bg-success"></div>
                <span className="text-sm">Autenticação e layout base implementados</span>
              </div>
              <div className="flex items-center gap-3">
                <div className="h-2 w-2 rounded-full bg-warning"></div>
                <span className="text-sm">Em desenvolvimento: CRUD de transações e categorias</span>
              </div>
              <div className="flex items-center gap-3">
                <div className="h-2 w-2 rounded-full bg-gray-300"></div>
                <span className="text-sm">Próximos: Assinaturas, metas e relatórios</span>
              </div>
            </div>
          </CardContent>
        </Card>
      </div>
    </MainLayout>
  )
}
