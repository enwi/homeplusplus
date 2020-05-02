import {Component, EventEmitter, Input, OnInit, Output} from '@angular/core';
import {FormBuilder, FormGroup} from '@angular/forms';
import {MatDialog} from '@angular/material/dialog';

import {DeleteDialog} from '../../delete-dialog/delete-dialog.component';
import {RuleCondition} from '../rule';

@Component({
  selector: 'app-editable-condition',
  templateUrl: './editable-condition.component.html',
  styleUrls: ['./editable-condition.component.css']
})
export class EditableConditionComponent implements OnInit {
  @Input() condition: RuleCondition;
  @Output() edit = new EventEmitter<RuleCondition>();
  @Output() delete = new EventEmitter<void>();

  editing = false;
  editForm: FormGroup;


  // Diese Bedingung wirklich löschen?
  private dialogText =
      $localize`Really delete this condition? This cannot be undone.`;

  constructor(private fb: FormBuilder, private dialog: MatDialog) {}

  ngOnInit() {}

  clickDelete() {
    const dialogRef =
        this.dialog.open(DeleteDialog, {data: {content: this.dialogText}});
    dialogRef.afterClosed().subscribe(result => {
      if (result) {
        this.delete.emit();
      }
    });
  }

  clickEdit() {
    this.editing = true;
    this.editForm =
        this.fb.group({condition: [JSON.stringify(this.condition)]});
  }
  finishEdit() {
    this.editing = false;
    const condition = JSON.parse(this.editForm.value.condition);
    this.edit.emit(condition);
  }
  cancelEdit() {
    this.editing = false;
  }
}
