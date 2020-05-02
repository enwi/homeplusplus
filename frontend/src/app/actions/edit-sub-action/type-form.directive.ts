import { Directive, ViewContainerRef } from '@angular/core';

@Directive({
  selector: 'app-type-form'
})
export class TypeFormDirective {

  constructor(public viewContainerRef: ViewContainerRef) { }

}
