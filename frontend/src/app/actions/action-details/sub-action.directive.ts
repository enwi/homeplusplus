import {ComponentFactoryResolver, Directive, EventEmitter, Input, OnInit, Output, Type, ViewContainerRef} from '@angular/core';

import {SubAction} from '../action';
import {SubActionView} from '../sub-actions/sub-action-view';

@Directive({selector: 'app-sub-action'})
export class SubActionDirective implements OnInit {
  @Input() subAction: SubAction;
  @Input() type: Type<SubActionView>;
  @Output() edit = new EventEmitter<void>();
  @Output() delete = new EventEmitter<void>();

  constructor(
      private viewContainerRef: ViewContainerRef,
      private componentFactoryResolver: ComponentFactoryResolver) {}

  ngOnInit() {
    this.updateView();
  }

  updateView() {
    this.viewContainerRef.clear();
    if (this.type && this.subAction) {
      const componentFactory =
          this.componentFactoryResolver.resolveComponentFactory(this.type);
      const componentRef =
          this.viewContainerRef.createComponent(componentFactory);
      componentRef.instance.initialize(this.subAction, this.edit, this.delete);
    }
  }
}
