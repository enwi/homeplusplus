export function notificationCategoryToString(category: number): string {
  if (category === 0) {
    return 'Information';
  } else if (category === 1) {
    return 'Warnung';
  } else {
    return 'Unbekannter Typ';
  }
}

export class Notification {
  category: number;
  message: string;
}
