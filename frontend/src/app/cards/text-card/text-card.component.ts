import { Component, OnInit, Input, Output, EventEmitter } from '@angular/core';

import { FormBuilder, FormGroup, Validators } from '@angular/forms';

@Component({
  selector: 'app-text-card',
  templateUrl: './text-card.component.html',
  styleUrls: [ './text-card.component.css',
    '../../styles/card.css' ]
})

export class TextCardComponent implements OnInit {
  @Input() header = 'unknown';
  @Input() value = 'unknown';
  @Output() change = new EventEmitter<string>();

  textcardFG: FormGroup;

  constructor(private form_builder: FormBuilder) { }

  ngOnInit() {
    this.textcardFG = this.form_builder.group({
      textcardInput: ['', Validators.required]
    });
  }

  onFormSubmit() {
    this.change.emit(this.textcardFG.value.textcardInput);
    this.textcardFG.reset();
  }

}
