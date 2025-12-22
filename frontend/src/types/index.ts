export interface User {
  id: string
  name: string
  email: string
  profileImage?: string
  partnerId?: string
  partner?: User
  createdAt: string
  updatedAt: string
}

export interface Category {
  id: string
  name: string
  icon: string
  color: string
  type: 'expense' | 'income'
  subcategories: string[]
  monthlyBudget?: number
  isDefault: boolean
}

export interface Transaction {
  id: string
  type: 'expense' | 'income'
  categoryId: string
  category?: Category
  subcategory?: string
  amount: number
  description: string
  date: string
  isRecurring: boolean
  recurringFreq?: 'daily' | 'weekly' | 'monthly' | 'yearly'
  recurringInterval?: number
  nextOccurrence?: string
  splitType: 'shared' | 'individual'
  splitPercentUser1?: number
  splitPercentUser2?: number
  paidById: string
  paidBy?: User
  attachments: string[]
  tags: string[]
  notes?: string
  createdAt: string
  updatedAt: string
}

export interface Subscription {
  id: string
  name: string
  categoryId: string
  category?: Category
  amount: number
  frequency: 'monthly' | 'quarterly' | 'semiannual' | 'yearly'
  billingDate: number
  nextBillingDate: string
  splitType: 'shared' | 'individual'
  splitPercent1?: number
  splitPercent2?: number
  paidById: string
  paidBy?: User
  isActive: boolean
  notifyBefore: number
  createdAt: string
  updatedAt: string
}

export interface Goal {
  id: string
  title: string
  description?: string
  targetAmount: number
  currentAmount: number
  deadline: string
  category: string
  icon: string
  color: string
  image?: string
  isShared: boolean
  monthlyContribution?: number
  progress: number
  userId: string
  user?: User
  createdAt: string
  updatedAt: string
}

export interface Notification {
  id: string
  userId: string
  type: 'bill' | 'budget' | 'goal' | 'subscription'
  priority: 'low' | 'medium' | 'high'
  title: string
  message: string
  icon: string
  isRead: boolean
  actionUrl?: string
  createdAt: string
}

export interface DashboardStats {
  monthlyIncome: number
  monthlyExpenses: number
  balance: number
  expensesByCategory: { category: string; amount: number; color: string }[]
  recentTransactions: Transaction[]
  upcomingBills: Subscription[]
  goalsProgress: Goal[]
}

export interface BudgetAlert {
  categoryId: string
  categoryName: string
  budgetAmount: number
  currentAmount: number
  percentage: number
  level: 'warning' | 'danger'
}

export interface PartnerBalance {
  user1Id: string
  user1Name: string
  user2Id: string
  user2Name: string
  user1Owes: number
  user2Owes: number
  netBalance: number
}
