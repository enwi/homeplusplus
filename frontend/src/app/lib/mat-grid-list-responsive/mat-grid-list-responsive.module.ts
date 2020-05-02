import {CommonModule} from '@angular/common';
import {NgModule} from '@angular/core';
import {MatGridListModule} from '@angular/material/grid-list';

import {MatGridListResponsiveDirective} from './mat-grid-list-responsive.directive';



@NgModule({
  imports: [CommonModule, MatGridListModule],
  declarations: [MatGridListResponsiveDirective],
  exports: [MatGridListResponsiveDirective]
})
export class MatGridListResponsiveModule {
}
