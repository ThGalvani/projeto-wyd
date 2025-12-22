import * as React from "react"
import { cn } from "@/lib/utils"
import { X } from "lucide-react"

interface ToastProps {
  id: string
  title?: string
  description?: string
  variant?: "default" | "destructive" | "success"
  onClose: () => void
}

const Toast: React.FC<ToastProps> = ({ title, description, variant = "default", onClose }) => {
  const variantClasses = {
    default: "bg-white border-gray-200",
    destructive: "bg-danger-50 border-danger-200 text-danger-900",
    success: "bg-success-50 border-success-200 text-success-900",
  }

  return (
    <div
      className={cn(
        "pointer-events-auto flex w-full max-w-md rounded-lg border p-4 shadow-lg",
        variantClasses[variant]
      )}
    >
      <div className="flex-1">
        {title && <div className="font-semibold">{title}</div>}
        {description && <div className="mt-1 text-sm opacity-90">{description}</div>}
      </div>
      <button
        onClick={onClose}
        className="ml-4 inline-flex h-6 w-6 items-center justify-center rounded-md hover:bg-black/10"
      >
        <X className="h-4 w-4" />
      </button>
    </div>
  )
}

export { Toast }
export type { ToastProps }
