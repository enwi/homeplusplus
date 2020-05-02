import {Type} from '@angular/core';

export interface DashboardCard {
  maxCols: number;
  maxRows: number;
  component: Type<any>;
  variables?: {[name: string]: any};
}
