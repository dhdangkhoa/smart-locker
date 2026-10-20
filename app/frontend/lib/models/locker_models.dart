enum CompartmentStatus {available, occupied, maintenance}

class CompartmentModel {
  final String id;
  final String boxNumber;
  final CompartmentStatus status;
  final String? trackingCode;
  final String? recipient;

  CompartmentModel({
    required this.id,
    required this.boxNumber,
    required this.status,
    this.trackingCode,
    this.recipient,
  });

}