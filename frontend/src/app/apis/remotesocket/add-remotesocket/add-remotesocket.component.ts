import { Component, OnInit } from '@angular/core';
import { RemoteSocketService } from '../remotesocket.service';
import { FormGroup, FormBuilder, Validators } from '@angular/forms';
import { Router, ActivatedRoute } from '@angular/router';

@Component({
  selector: 'app-add-remotesocket',
  templateUrl: './add-remotesocket.component.html',
  styleUrls: ['./add-remotesocket.component.css']
})
export class AddRemotesocketComponent implements OnInit {

  form: FormGroup;

  constructor(private remoteSocketService: RemoteSocketService,
    private fb: FormBuilder,
    private router: Router,
    private route: ActivatedRoute) { }

  ngOnInit() {
    this.form = this.fb.group({
      name: ['', Validators.required],
      location: [''],
      code: ['', Validators.pattern('[01]{5}[1-4]')]
    });
  }

  submit() {
    if (this.form.valid) {
      
    }
  }
  cancel() {
    this.router.navigate(['../'], { relativeTo: this.route });
  }

}
